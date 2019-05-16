//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher, Jordi Cotela
//
// See Master-Thesis P.Bucher
// "Development and Implementation of a Parallel
//  Framework for Non-Matching Grid Mapping"

#if !defined(KRATOS_BARYCENTRIC_MAPPER_H_INCLUDED )
#define  KRATOS_BARYCENTRIC_MAPPER_H_INCLUDED

// System includes

// External includes

// Project includes
#include "interpolative_mapper_base.h"


namespace Kratos
{
///@name Kratos Classes
///@{

class BarycentricInterfaceInfo : public MapperInterfaceInfo
{
public:
    explicit BarycentricInterfaceInfo(std::size_t NumInterpolationNodes) {
        mNodeIds.resize(NumInterpolationNodes);
        mNeighborCoordinates.resize(3*NumInterpolationNodes);
        std::fill(mNodeIds.begin(), mNodeIds.end(), -1);
        std::fill(mNeighborCoordinates.begin(), mNeighborCoordinates.end(), std::numeric_limits<double>::max());
    }

    explicit BarycentricInterfaceInfo(const CoordinatesArrayType& rCoordinates,
                                      const IndexType SourceLocalSystemIndex,
                                      const IndexType SourceRank,
                                      const std::size_t NumInterpolationNodes)
        : MapperInterfaceInfo(rCoordinates, SourceLocalSystemIndex, SourceRank) {
        mNodeIds.resize(NumInterpolationNodes);
        mNeighborCoordinates.resize(3*NumInterpolationNodes);
        std::fill(mNodeIds.begin(), mNodeIds.end(), -1);
        std::fill(mNeighborCoordinates.begin(), mNeighborCoordinates.end(), std::numeric_limits<double>::max());
    }

    MapperInterfaceInfo::Pointer Create() const override
    {
        return Kratos::make_shared<BarycentricInterfaceInfo>(mNodeIds.size());
    }

    MapperInterfaceInfo::Pointer Create(const CoordinatesArrayType& rCoordinates,
                                        const IndexType SourceLocalSystemIndex,
                                        const IndexType SourceRank) const override
    {
        return Kratos::make_shared<BarycentricInterfaceInfo>(
            rCoordinates,
            SourceLocalSystemIndex,
            SourceRank,
            mNodeIds.size());
    }

    InterfaceObject::ConstructionType GetInterfaceObjectType() const override
    {
        return InterfaceObject::ConstructionType::Node_Coords;
    }

    void ProcessSearchResult(const InterfaceObject& rInterfaceObject,
                             const double NeighborDistance) override;

    void GetValue(std::vector<int>& rValue,
                  const InfoType ValueType) const override
    {
        rValue = mNodeIds;
    }

    void GetValue(std::vector<double>& rValue,
                  const InfoType ValueType) const override
    {
        rValue = mNeighborCoordinates;
    }

private:

    std::vector<int> mNodeIds;
    std::vector<double> mNeighborCoordinates;

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, MapperInterfaceInfo );
        rSerializer.save("NodeIds", mNodeIds);
        rSerializer.save("NeighborCoords", mNeighborCoordinates);
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, MapperInterfaceInfo );
        rSerializer.load("NodeIds", mNodeIds);
        rSerializer.load("NeighborCoords", mNeighborCoordinates);
    }
};

class BarycentricLocalSystem : public MapperLocalSystem
{
public:

    explicit BarycentricLocalSystem(NodePointerType pNode) : mpNode(pNode) {}

    void CalculateAll(MatrixType& rLocalMappingMatrix,
                      EquationIdVectorType& rOriginIds,
                      EquationIdVectorType& rDestinationIds,
                      MapperLocalSystem::PairingStatus& rPairingStatus) const override;

    CoordinatesArrayType& Coordinates() const override
    {
        KRATOS_DEBUG_ERROR_IF_NOT(mpNode) << "Members are not intitialized!" << std::endl;
        return mpNode->Coordinates();
    }

    /// Turn back information as a string.
    std::string PairingInfo(const int EchoLevel) const override;

private:
    NodePointerType mpNode;

};

/// Nearest Neighbor Mapper
/** This class implements the Nearest Neighbor Mapping technique.
* Each node on the destination side gets assigned is's closest neighbor on the other side of the interface.
* In the mapping phase every node gets assigned the value of it's neighbor
* For information abt the available echo_levels and the JSON default-parameters
* look into the class description of the MapperCommunicator
*/
template<class TSparseSpace, class TDenseSpace>
class BarycentricMapper : public InterpolativeMapperBase<TSparseSpace, TDenseSpace>
{
public:
    ///@name Type Definitions
    ///@{

    ///@}
    ///@name Pointer Definitions
    /// Pointer definition of BarycentricMapper
    KRATOS_CLASS_POINTER_DEFINITION(BarycentricMapper);

    typedef InterpolativeMapperBase<TSparseSpace, TDenseSpace> BaseType;
    typedef typename BaseType::MapperUniquePointerType MapperUniquePointerType;
    typedef typename BaseType::MapperInterfaceInfoUniquePointerType MapperInterfaceInfoUniquePointerType;

    ///@}
    ///@name Life Cycle
    ///@{

    // Default constructor, needed for registration
    BarycentricMapper(ModelPart& rModelPartOrigin,
                          ModelPart& rModelPartDestination)
                          : BaseType(rModelPartOrigin, rModelPartDestination) {}

    BarycentricMapper(ModelPart& rModelPartOrigin,
                          ModelPart& rModelPartDestination,
                          Parameters JsonParameters)
                          : BaseType(rModelPartOrigin,
                                     rModelPartDestination,
                                     JsonParameters)
    {
        this->ValidateInput();

        const std::string interpolation_type = JsonParameters["interpolation_type"].GetString();
        if (interpolation_type == "line") {
            mNumInterpolationNodes = 2;
        } else if (interpolation_type == "triangle") {
            mNumInterpolationNodes = 3;
        } else if (interpolation_type == "tetrahedra") {
            mNumInterpolationNodes = 4;
        } else {
            KRATOS_ERROR << "BarycentricMapper: No \"interpolation_type\" was specified, please select \"line\", \"triangle\" or \"tetrahedra\"" << std::endl;
        }

        this->Initialize();
    }

    /// Destructor.
    ~BarycentricMapper() override = default;

    ///@}
    ///@name Operations
    ///@{

    MapperUniquePointerType Clone(ModelPart& rModelPartOrigin,
                                  ModelPart& rModelPartDestination,
                                  Parameters JsonParameters) const override
    {
        return Kratos::make_unique<BarycentricMapper<TSparseSpace, TDenseSpace>>(
            rModelPartOrigin,
            rModelPartDestination,
            JsonParameters);
    }

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "BarycentricMapper";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "BarycentricMapper; working in: ";
        if (TSparseSpace::IsDistributed()){
            rOStream << "MPI";
        } else {
            rOStream << "OpenMP";
        }
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        BaseType::PrintData(rOStream);
    }

private:
    ///@name Member Variables
    ///@{

    std::size_t mNumInterpolationNodes;

    ///@}

    ///@name Private Operations
    ///@{

    void CreateMapperLocalSystems(
        const Communicator& rModelPartCommunicator,
        std::vector<Kratos::unique_ptr<MapperLocalSystem>>& rLocalSystems) override
    {
        MapperUtilities::CreateMapperLocalSystemsFromNodes<BarycentricLocalSystem>(
            rModelPartCommunicator,
            rLocalSystems);
    }

    MapperInterfaceInfoUniquePointerType GetMapperInterfaceInfo() const override
    {
        return Kratos::make_unique<BarycentricInterfaceInfo>(mNumInterpolationNodes);
    }

    Parameters GetMapperDefaultSettings() const override
    {
        return Parameters( R"({
            "search_radius"            : -1.0,
            "search_iterations"        : 3,
            "interpolation_type"       : "unspecified",
            "echo_level"               : 0
        })");
    }

    ///@}

}; // Class BarycentricMapper

///@} addtogroup block
}  // namespace Kratos.

#endif // KRATOS_BARYCENTRIC_MAPPER_H_INCLUDED  defined
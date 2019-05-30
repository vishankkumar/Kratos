// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Riccardo Rossi
//                   Vicente Mataix Ferrandiz
//

// System includes
#if !defined(KRATOS_LINE_LOAD_CONDITION_2D_H_INCLUDED )
#define  KRATOS_LINE_LOAD_CONDITION_2D_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_conditions/base_load_condition.h"

namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/**
 * @class LineLoadCondition2D
 * @ingroup StructuralMechanicsApplication
 * @brief This class is the responsible to add the contributions of the RHS and LHS of the line loads of the structure
 * @details It allows to consider different types of pressure and line loads
 * @author Riccardo Rossi
 * @author Vicente Mataix Ferrandiz
 */
class KRATOS_API(STRUCTURAL_MECHANICS_APPLICATION) LineLoadCondition2D
    : public BaseLoadCondition
{
public:
    ///@name Type Definitions
    ///@{

    /// We define the base class BaseLoadCondition
    typedef BaseLoadCondition BaseType;

    /// Dfinition of the index type
    typedef BaseType::IndexType IndexType;

    /// Definition of the size type
    typedef BaseType::SizeType SizeType;

    /// Definition of the node type
    typedef BaseType::NodeType NodeType;

    /// Definition of the properties type
    typedef BaseType::PropertiesType PropertiesType;

    /// Definition of the geometry type with given NodeType
    typedef BaseType::GeometryType GeometryType;

    /// Definition of nodes container type, redefined from GeometryType
    typedef BaseType::NodesArrayType NodesArrayType;

    /// Counted pointer of LineLoadCondition2D
    KRATOS_CLASS_INTRUSIVE_POINTER_DEFINITION( LineLoadCondition2D );

    ///@}
    ///@name Life Cycle
    ///@{

    // Constructor using an array of nodes
    LineLoadCondition2D(
        IndexType NewId,
        GeometryType::Pointer pGeometry
        );

    // Constructor using an array of nodes with properties
    LineLoadCondition2D(
        IndexType NewId,
        GeometryType::Pointer pGeometry,
        PropertiesType::Pointer pProperties
        );

    /// Destructor.
    ~LineLoadCondition2D() override;

    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Creates a new condition pointer
     * @param NewId the ID of the new condition
     * @param ThisNodes the nodes of the new condition
     * @param pProperties the properties assigned to the new condition
     * @return a Pointer to the new condition
     */
    Condition::Pointer Create(
        IndexType NewId,
        NodesArrayType const& ThisNodes,
        PropertiesType::Pointer pProperties
        ) const override;

    /**
     * @brief Creates a new condition pointer
     * @param NewId the ID of the new condition
     * @param pGeom the geometry to be employed
     * @param pProperties the properties assigned to the new condition
     * @return a Pointer to the new condition
     */
    Condition::Pointer Create(
        IndexType NewId,
        GeometryType::Pointer pGeom,
        PropertiesType::Pointer pProperties
        ) const override;

    /**
     * @brief Creates a new condition pointer and clones the previous condition data
     * @param NewId the ID of the new condition
     * @param ThisNodes the nodes of the new condition
     * @return a Pointer to the new condition
     */
    Condition::Pointer Clone (
        IndexType NewId,
        NodesArrayType const& ThisNodes
        ) const override;

    /**
     * @brief Get on rVariable a array_1d Value
     * @param rVariable Internal values
     * @param rCurrentProcessInfo The current process information
     * @param rOutput The values of interest (array_1d)
     */
    void GetValueOnIntegrationPoints(
        const Variable<array_1d<double, 3 > >& rVariable,
        std::vector<array_1d<double, 3 > >& rOutput,
        const ProcessInfo& rCurrentProcessInfo
        ) override;

    /**
     * @brief Calculate a array_1d Variable
     * @param rVariable Internal values
     * @param rCurrentProcessInfo The current process information
     * @param rOutput The values of interest (array_1d)
     */
    void CalculateOnIntegrationPoints(
        const Variable<array_1d<double, 3 > >& rVariable,
        std::vector< array_1d<double, 3 > >& rOutput,
        const ProcessInfo& rCurrentProcessInfo
        ) override;

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "LineLoadCondition2D #" << Id();
        return buffer.str();
    }

    /// Print information about this object.

    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "LineLoadCondition2D #" << Id();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        pGetGeometry()->PrintData(rOStream);
    }

    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{


    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{

    /**
     * @brief This functions calculates both the RHS and the LHS
     * @param rLeftHandSideMatrix: The LHS
     * @param rRightHandSideVector: The RHS
     * @param rCurrentProcessInfo: The current process info instance
     * @param CalculateStiffnessMatrixFlag: The flag to set if compute the LHS
     * @param CalculateResidualVectorFlag: The flag to set if compute the RHS
     */
    void CalculateAll(
        MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        const ProcessInfo& rCurrentProcessInfo,
        const bool CalculateStiffnessMatrixFlag,
        const bool CalculateResidualVectorFlag
        ) override;

    /**
     * @brief This method adds the local contribution of the pressure to the LHS matrix
     * @param rK The local LHS contribution
     * @param rDN_De The local gradient of the geometry
     * @param rN The shape function of the current integration point
     * @param Pressure The pressure to be applied
     * @param Weight The integration contribution
     */
    void CalculateAndSubKp(
        Matrix& rK,
        const Matrix& rDN_De,
        const Vector& rN,
        const double Pressure,
        const double IntegrationWeight
        ) const;

    /**
     * @brief This method adds the pressure contribution to the RHS
     * @param rResidualVector The local contribution to the RHS
     * @param rN The corresponding shape function
     * @param rNormal The normal to the geometry surface
     * @param Pressure The pressure to be applied
     * @param Weight The integration contribution
     * @param rCurrentProcessInfo The current instance of process info
     */
    void CalculateAndAddPressureForce(
        VectorType& rRightHandSideVector,
        const Vector& rN,
        const array_1d<double, 3>& rNormal,
        const double Pressure,
        const double IntegrationWeight
        ) const;

    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{

    // A protected default constructor necessary for serialization
    LineLoadCondition2D() {};

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save( Serializer& rSerializer ) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, BaseLoadCondition );
    }

    void load( Serializer& rSerializer ) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, BaseLoadCondition );
    }


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    //LineLoadCondition2D& operator=(const LineLoadCondition2D& rOther);

    /// Copy constructor.
    //LineLoadCondition2D(const LineLoadCondition2D& rOther);


    ///@}

}; // Class LineLoadCondition2D

///@}
///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
        LineLoadCondition2D& rThis);
/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
        const LineLoadCondition2D& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

///@}

}  // namespace Kratos.

#endif // KRATOS_LINE_LOAD_CONDITION_2D_H_INCLUDED  defined



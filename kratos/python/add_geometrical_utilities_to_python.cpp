//
//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//


// System includes

// External includes

// Project includes
#include "includes/define_python.h"
#include "processes/process.h"
#include "python/add_geometrical_utilities_to_python.h"

//Geometrical (and kernel) utilities
#include "utilities/normal_calculation_utils.h"
#include "utilities/body_normal_calculation_utils.h"
#include "utilities/body_distance_calculation_utils.h"
#include "utilities/signed_distance_calculation_utils.h"
#include "utilities/parallel_levelset_distance_calculator.h"
#include "utilities/brute_force_point_locator.h"
#include "utilities/deflation_utils.h"
#include "utilities/binbased_fast_point_locator.h"
#include "utilities/binbased_fast_point_locator_conditions.h"
#include "utilities/binbased_nodes_in_element_locator.h"
#include "utilities/embedded_skin_utility.h"
#include "utilities/geometry_tester.h"
#include "utilities/cutting_utility.h"
#include "utilities/geometrical_transformation_utilities.h"
#include "utilities/openmp_utils.h"
#include "utilities/iso_printer.h"
#include "utilities/activation_utilities.h"
#include "utilities/interval_utility.h"
#include "utilities/table_stream_utility.h"
#include "utilities/read_materials_utility.h"
#include "utilities/sensitivity_builder.h"


//new
#include "utilities/delaunator_utilities.h"
#include "utilities/compare_elements_and_conditions_utility.h"




namespace Kratos {
namespace Python {

/**
 * @brief Sets the current table utility on the process info
 * @param rCurrentProcessInfo The process info
 */
void SetOnProcessInfo(
    typename TableStreamUtility::Pointer pTable,
    ProcessInfo& rCurrentProcessInfo
    )
{
    rCurrentProcessInfo[TABLE_UTILITY] = pTable;
}

// Embedded skin utility auxiliar functions
template<std::size_t TDim>
void InterpolateMeshVariableToSkinDouble(
    EmbeddedSkinUtility<TDim> &rEmbeddedSkinUtility,
    const Variable<double> &rVariable,
    const Variable<double> &rEmbeddedVariable)
{
    rEmbeddedSkinUtility.InterpolateMeshVariableToSkin(rVariable, rEmbeddedVariable);
}

template<std::size_t TDim>
void InterpolateMeshVariableToSkinArray(
    EmbeddedSkinUtility<TDim> &rEmbeddedSkinUtility,
    const Variable<array_1d<double,3>> &rVariable,
    const Variable<array_1d<double,3>> &rEmbeddedVariable)
{
    rEmbeddedSkinUtility.InterpolateMeshVariableToSkin(rVariable, rEmbeddedVariable);
}

template<std::size_t TDim>
void InterpolateDiscontinuousMeshVariableToSkinDouble(
    EmbeddedSkinUtility<TDim> &rEmbeddedSkinUtility,
    const Variable<double> &rVariable,
    const Variable<double> &rEmbeddedVariable,
    const std::string &rInterfaceSide)
{
    rEmbeddedSkinUtility.InterpolateDiscontinuousMeshVariableToSkin(rVariable, rEmbeddedVariable, rInterfaceSide);
}

template<std::size_t TDim>
void InterpolateDiscontinuousMeshVariableToSkinArray(
    EmbeddedSkinUtility<TDim> &rEmbeddedSkinUtility,
    const Variable<array_1d<double,3>> &rVariable,
    const Variable<array_1d<double,3>> &rEmbeddedVariable,
    const std::string &rInterfaceSide)
{
    rEmbeddedSkinUtility.InterpolateDiscontinuousMeshVariableToSkin(rVariable, rEmbeddedVariable, rInterfaceSide);
}

// Parallel distance calculator
void CalculateDistancesDefault2D(ParallelDistanceCalculator<2>& rParallelDistanceCalculator,ModelPart& rModelPart, const Variable<double>& rDistanceVar, const Variable<double>& rAreaVar, const unsigned int max_levels, const double max_distance)
{
    rParallelDistanceCalculator.CalculateDistances(rModelPart, rDistanceVar, rAreaVar, max_levels, max_distance);
}

void CalculateDistancesFlag2D(ParallelDistanceCalculator<2>& rParallelDistanceCalculator, ModelPart& rModelPart, const Variable<double>& rDistanceVar, const Variable<double>& rAreaVar, const unsigned int max_levels, const double max_distance, Flags Options)
{
    rParallelDistanceCalculator.CalculateDistances(rModelPart, rDistanceVar, rAreaVar, max_levels, max_distance, Options);
}

void CalculateDistancesDefault3D(ParallelDistanceCalculator<3>& rParallelDistanceCalculator,ModelPart& rModelPart, const Variable<double>& rDistanceVar, const Variable<double>& rAreaVar, const unsigned int max_levels, const double max_distance)
{
    rParallelDistanceCalculator.CalculateDistances(rModelPart, rDistanceVar, rAreaVar, max_levels, max_distance);
}

void CalculateDistancesFlag3D(ParallelDistanceCalculator<3>& rParallelDistanceCalculator, ModelPart& rModelPart, const Variable<double>& rDistanceVar, const Variable<double>& rAreaVar, const unsigned int max_levels, const double max_distance, Flags Options)
{
    rParallelDistanceCalculator.CalculateDistances(rModelPart, rDistanceVar, rAreaVar, max_levels, max_distance, Options);
}



//compare elements and conditions utilities
std::string GetRegisteredNameElement(const Element& rElement)
{
    std::string name;
    CompareElementsAndConditionsUtility::GetRegisteredName(rElement, name);
    return name;
}

std::string GetRegisteredNameCondition(const Condition& rCondition)
{
    std::string name;
    CompareElementsAndConditionsUtility::GetRegisteredName(rCondition, name);
    return name;
}





    void AddGeometricalUtilitiesToPython(pybind11::module &m) 
{

    namespace py = pybind11; 

    py::class_<DeflationUtils>(m,"DeflationUtils")
        .def(py::init<>())
        .def("VisualizeAggregates",&DeflationUtils::VisualizeAggregates)
        ;
    
    
    // This is required to recognize the different overloads of NormalCalculationUtils::CalculateOnSimplex
    typedef  void (NormalCalculationUtils::*CalcOnSimplexCondType)(NormalCalculationUtils::ConditionsArrayType&,int);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexMPType)(ModelPart&,int);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexWithDoubleVarType)(ModelPart&,int,Variable<double>&);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexWithIntVarType)(ModelPart&,int,Variable<int>&);
//            typedef  void (NormalCalculationUtils::*CalcOnSimplexWithArrayVarType)(ModelPart&,int,Variable< array_1d<double,3> >&,const array_1d<double,3>&);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexWithDoubleVarAlphaType)(ModelPart&,int,Variable<double>&,const double,const double);

    CalcOnSimplexCondType CalcOnSimplex_Cond = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexMPType CalcOnSimplex_ModelPart = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexWithDoubleVarType CalcOnSimplexWithDoubleVar = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexWithIntVarType CalcOnSimplexWithIntVar = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexWithDoubleVarAlphaType CalcOnSimplexWithDoubleVarAlpha = &NormalCalculationUtils::CalculateOnSimplex;

    py::class_<NormalCalculationUtils > (m,"NormalCalculationUtils")
        .def(py::init<>())
        .def("CalculateOnSimplex", CalcOnSimplex_Cond)
        .def("CalculateOnSimplex", CalcOnSimplex_ModelPart)
        .def("CalculateOnSimplex", CalcOnSimplexWithDoubleVar)
        .def("CalculateOnSimplex", CalcOnSimplexWithIntVar)
        .def("CalculateOnSimplex", CalcOnSimplexWithDoubleVarAlpha)
        .def("SwapNormals", &NormalCalculationUtils::SwapNormals)
    //                    .def("CalculateOnSimplex", CalcOnSimplexWithArrayVar)
        ;

    py::class_<BodyNormalCalculationUtils > (m,"BodyNormalCalculationUtils")
        .def(py::init<>())
        .def("CalculateBodyNormals", &BodyNormalCalculationUtils::CalculateBodyNormals)
        ;

    py::class_<BodyDistanceCalculationUtils > (m,"BodyDistanceCalculationUtils")
        .def(py::init<>())
        .def("CalculateDistances2D", &BodyDistanceCalculationUtils::CalculateDistances < 2 >)
        .def("CalculateDistances3D", &BodyDistanceCalculationUtils::CalculateDistances < 3 >)
        ;

    py::class_<SignedDistanceCalculationUtils < 2 > >(m,"SignedDistanceCalculationUtils2D")
        .def(py::init<>())
        .def("CalculateDistances", &SignedDistanceCalculationUtils < 2 > ::CalculateDistances)
        .def("FindMaximumEdgeSize", &SignedDistanceCalculationUtils < 2 > ::FindMaximumEdgeSize)
        ;

    py::class_<SignedDistanceCalculationUtils < 3 > >(m,"SignedDistanceCalculationUtils3D")
        .def(py::init<>())
        .def("CalculateDistances", &SignedDistanceCalculationUtils < 3 > ::CalculateDistances)
        .def("FindMaximumEdgeSize", &SignedDistanceCalculationUtils < 3 > ::FindMaximumEdgeSize)
        ;

    py::class_<ParallelDistanceCalculator < 2 > >(m,"ParallelDistanceCalculator2D")
        .def(py::init<>())
        .def("CalculateDistances", CalculateDistancesDefault2D)
        .def("CalculateDistances", CalculateDistancesFlag2D)
        .def("CalculateInterfacePreservingDistances", &ParallelDistanceCalculator < 2 > ::CalculateInterfacePreservingDistances)
        .def("CalculateDistancesLagrangianSurface", &ParallelDistanceCalculator < 2 > ::CalculateDistancesLagrangianSurface)
        .def("FindMaximumEdgeSize", &ParallelDistanceCalculator < 2 > ::FindMaximumEdgeSize)
        .def_readonly_static("CALCULATE_EXACT_DISTANCES_TO_PLANE", &ParallelDistanceCalculator<2>::CALCULATE_EXACT_DISTANCES_TO_PLANE)
        ;

    py::class_<ParallelDistanceCalculator < 3 > >(m,"ParallelDistanceCalculator3D")
        .def(py::init<>())
        .def("CalculateDistances", CalculateDistancesDefault3D)
        .def("CalculateDistances", CalculateDistancesFlag3D)
        .def("CalculateInterfacePreservingDistances", &ParallelDistanceCalculator < 3 > ::CalculateInterfacePreservingDistances)
        .def("CalculateDistancesLagrangianSurface", &ParallelDistanceCalculator < 3 > ::CalculateDistancesLagrangianSurface)
        .def("FindMaximumEdgeSize", &ParallelDistanceCalculator < 3 > ::FindMaximumEdgeSize)
        .def_readonly_static("CALCULATE_EXACT_DISTANCES_TO_PLANE", &ParallelDistanceCalculator<3>::CALCULATE_EXACT_DISTANCES_TO_PLANE)
        ;

    //brute force point locator
    py::class_<BruteForcePointLocator> (m, "BruteForcePointLocator")
        .def(py::init<ModelPart& >())
        .def("FindNode", &BruteForcePointLocator::FindNode)
        .def("FindElement", &BruteForcePointLocator::FindElement)
        .def("FindCondition", &BruteForcePointLocator::FindCondition)
        ;
    

    //isoprinter
    py::class_<IsosurfacePrinterApplication >(m,"IsosurfacePrinterApplication")
        .def(py::init<ModelPart& >() )
        .def("AddScalarVarIsosurface", &IsosurfacePrinterApplication::AddScalarVarIsosurface)
        .def("AddScalarVarIsosurfaceAndLower", &IsosurfacePrinterApplication::AddScalarVarIsosurfaceAndLower)
        .def("AddScalarVarIsosurfaceAndHigher", &IsosurfacePrinterApplication::AddScalarVarIsosurfaceAndHigher)
        .def("ClearData", &IsosurfacePrinterApplication::ClearData)
        .def("AddSkinConditions", &IsosurfacePrinterApplication::AddSkinConditions)
        .def("CreateNodesArray", &IsosurfacePrinterApplication::CreateNodesArray)
        ;

    //signed distance calculator
//     py::class_<SignedDistanceCalculationBinBased<2> >(m,"SignedDistanceCalculationBinBased2D", init<>())
//             .def("CalculateDistances",&SignedDistanceCalculationBinBased<2>::CalculateDistances )
//                             .def("FindMaximumEdgeSize",&SignedDistanceCalculationBinBased<2>::FindMaximumEdgeSize )
//             ;
//
//     py::class_<SignedDistanceCalculationBinBased<3> >(m,"SignedDistanceCalculationBinBased3D", init<>())
//             .def("CalculateDistances",&SignedDistanceCalculationBinBased<3>::CalculateDistances )
//                             .def("FindMaximumEdgeSize",&SignedDistanceCalculationBinBased<3>::FindMaximumEdgeSize )
//             ;    


    //binbased locators
    py::class_< BinBasedFastPointLocator < 2 >, BinBasedFastPointLocator < 2 >::Pointer >(m,"BinBasedFastPointLocator2D")
        .def(py::init<ModelPart& >())
        .def("UpdateSearchDatabase", &BinBasedFastPointLocator < 2 > ::UpdateSearchDatabase)
        .def("UpdateSearchDatabaseAssignedSize", &BinBasedFastPointLocator < 2 > ::UpdateSearchDatabaseAssignedSize)
        .def("FindPointOnMesh", &BinBasedFastPointLocator < 2 > ::FindPointOnMeshSimplified)
        ;

    py::class_< BinBasedFastPointLocator < 3 >, BinBasedFastPointLocator < 3 >::Pointer >(m,"BinBasedFastPointLocator3D")
        .def(py::init<ModelPart&  >())
        .def("UpdateSearchDatabase", &BinBasedFastPointLocator < 3 > ::UpdateSearchDatabase)
        .def("FindPointOnMesh", &BinBasedFastPointLocator < 3 > ::FindPointOnMeshSimplified)
        .def("UpdateSearchDatabaseAssignedSize", &BinBasedFastPointLocator < 3 > ::UpdateSearchDatabaseAssignedSize)
        ;

    py::class_< BinBasedFastPointLocatorConditions < 2 > >(m,"BinBasedFastPointLocatorConditions2D")
        .def(py::init<ModelPart& >())
        .def("UpdateSearchDatabase", &BinBasedFastPointLocatorConditions < 2 > ::UpdateSearchDatabase)
        .def("UpdateSearchDatabaseAssignedSize", &BinBasedFastPointLocatorConditions < 2 > ::UpdateSearchDatabaseAssignedSize)
        .def("FindPointOnMesh", &BinBasedFastPointLocatorConditions < 2 > ::FindPointOnMeshSimplified)
        ;

    py::class_< BinBasedFastPointLocatorConditions < 3 > >(m,"BinBasedFastPointLocatorConditions3D")
        .def(py::init<ModelPart&  >())
        .def("UpdateSearchDatabase", &BinBasedFastPointLocatorConditions < 3 > ::UpdateSearchDatabase)
        .def("FindPointOnMesh", &BinBasedFastPointLocatorConditions < 3 > ::FindPointOnMeshSimplified)
        .def("UpdateSearchDatabaseAssignedSize", &BinBasedFastPointLocatorConditions < 3 > ::UpdateSearchDatabaseAssignedSize)
        ;

    py::class_< BinBasedNodesInElementLocator < 2 > >(m,"BinBasedNodesInElementLocator2D")
        .def(py::init<ModelPart& >())
        .def("UpdateSearchDatabase", &BinBasedNodesInElementLocator < 2 > ::UpdateSearchDatabase)
        .def("FindNodesInElement", &BinBasedNodesInElementLocator < 2 > ::FindNodesInElement)
        .def("UpdateSearchDatabaseAssignedSize", &BinBasedNodesInElementLocator < 2 > ::UpdateSearchDatabaseAssignedSize)
        ;

    py::class_< BinBasedNodesInElementLocator < 3 > >(m,"BinBasedNodesInElementLocator3D")
        .def(py::init<ModelPart&  >())
        .def("UpdateSearchDatabase", &BinBasedNodesInElementLocator < 3 > ::UpdateSearchDatabase)
        .def("FindNodesInElement", &BinBasedNodesInElementLocator < 3 > ::FindNodesInElement)
        .def("UpdateSearchDatabaseAssignedSize", &BinBasedNodesInElementLocator < 3 > ::UpdateSearchDatabaseAssignedSize)
        ;    

    
    //embeded skin utilities
    py::class_< EmbeddedSkinUtility < 2 > >(m,"EmbeddedSkinUtility2D")
        .def(py::init< ModelPart&, ModelPart&, const std::string >())
        .def("GenerateSkin", &EmbeddedSkinUtility < 2 > ::GenerateSkin)
        .def("InterpolateMeshVariableToSkin", InterpolateMeshVariableToSkinArray< 2 > )
        .def("InterpolateMeshVariableToSkin", InterpolateMeshVariableToSkinDouble< 2 > )
        .def("InterpolateDiscontinuousMeshVariableToSkin", InterpolateDiscontinuousMeshVariableToSkinArray< 2 > )
        .def("InterpolateDiscontinuousMeshVariableToSkin", InterpolateDiscontinuousMeshVariableToSkinDouble< 2 > )
        ;

    py::class_< EmbeddedSkinUtility <3 > >(m,"EmbeddedSkinUtility3D")
        .def(py::init< ModelPart&, ModelPart&, const std::string >())
        .def("GenerateSkin", &EmbeddedSkinUtility < 3 > ::GenerateSkin)
        .def("InterpolateMeshVariableToSkin", InterpolateMeshVariableToSkinArray< 3 > )
        .def("InterpolateMeshVariableToSkin", InterpolateMeshVariableToSkinDouble< 3 > )
        .def("InterpolateDiscontinuousMeshVariableToSkin", InterpolateDiscontinuousMeshVariableToSkinArray< 3 > )
        .def("InterpolateDiscontinuousMeshVariableToSkin", InterpolateDiscontinuousMeshVariableToSkinDouble< 3 > )
        ;  

    //Geometry tester
    py::class_< GeometryTesterUtility>(m,"GeometryTesterUtility")
        .def(py::init< >())
        .def("RunTest", &GeometryTesterUtility::RunTest)
        .def("TestTriangle2D3N", &GeometryTesterUtility::TestTriangle2D3N)
        .def("TestTriangle2D6N", &GeometryTesterUtility::TestTriangle2D6N)
        .def("TestTetrahedra3D4N", &GeometryTesterUtility::TestTetrahedra3D4N)
        .def("TestTetrahedra3D10N", &GeometryTesterUtility::TestTetrahedra3D10N)
        .def("TestHexahedra3D8N", &GeometryTesterUtility::TestHexahedra3D8N)
        .def("TestHexahedra3D27N", &GeometryTesterUtility::TestHexahedra3D27N)
        .def("TestHexahedra3D20N", &GeometryTesterUtility::TestHexahedra3D20N)
        .def("TestQuadrilateralInterface2D4N", &GeometryTesterUtility::TestQuadrilateralInterface2D4N)
        .def("TestPrismInterface3D6N", &GeometryTesterUtility::TestPrismInterface3D6N)
        .def("TestHexahedraInterface3D8N", &GeometryTesterUtility::TestHexahedraInterface3D8N)
        ;

    //Cutting utility
    py::class_<CuttingUtility >(m,"CuttingUtility")
        .def(py::init< >())
        .def("GenerateCut", &CuttingUtility::GenerateCut)
        .def("UpdateCutData", &CuttingUtility ::UpdateCutData)
        .def("AddSkinConditions", &CuttingUtility ::AddSkinConditions)
        .def("AddVariablesToCutModelPart", &CuttingUtility::AddVariablesToCutModelPart )
        .def("FindSmallestEdge", &CuttingUtility ::FindSmallestEdge)
        ;     


    //interval utility    
    py::class_<IntervalUtility >(m,"IntervalUtility")
        .def(py::init<Parameters >())
        .def("GetIntervalBegin", &IntervalUtility::GetIntervalBegin)
        .def("GetIntervalEnd", &IntervalUtility::GetIntervalEnd)
        .def("IsInInterval", &IntervalUtility ::IsInInterval)
        ; 
    
    // Adding table from table stream to python
    py::class_<TableStreamUtility, typename TableStreamUtility::Pointer>(m,"TableStreamUtility")
        .def(py::init<>())
        .def(py::init< bool >())
        .def("SetOnProcessInfo",SetOnProcessInfo)
        ;
    
    
    // Read materials utility
    py::class_<ReadMaterialsUtility, typename ReadMaterialsUtility::Pointer>(m, "ReadMaterialsUtility")
    .def(py::init<Model&>())
    .def(py::init<Parameters, Model&>())
    .def("ReadMaterials",&ReadMaterialsUtility::ReadMaterials)
    ;


    //sensitivity builder
    py::class_<SensitivityBuilder>(m, "SensitivityBuilder")
        .def(py::init<Parameters, ModelPart&, AdjointResponseFunction::Pointer>())
        .def("Initialize", &SensitivityBuilder::Initialize)
        .def("UpdateSensitivities", &SensitivityBuilder::UpdateSensitivities);

    
    //OpenMP utilities
    py::class_<OpenMPUtils >(m,"OpenMPUtils")
        .def(py::init<>())
        .def_static("SetNumThreads", &OpenMPUtils::SetNumThreads)
    //     .staticmethod("SetNumThreads")
        .def_static("GetNumThreads", &OpenMPUtils::GetNumThreads)
    //     .staticmethod("GetNumThreads")
        .def_static("PrintOMPInfo", &OpenMPUtils::PrintOMPInfo)
    //     .staticmethod("PrintOMPInfo")
        ;

    //activation utilities
    py::class_< ActivationUtilities >(m,"ActivationUtilities")
        .def(py::init< >())
        .def("ActivateElementsAndConditions", &ActivationUtilities::ActivateElementsAndConditions)
        ;    



    // Delaunator utilities
    auto mod_delaunator = m.def_submodule("CreateTriangleMeshFromNodes");
    mod_delaunator.def("CreateTriangleMeshFromNodes",&DelaunatorUtilities::CreateTriangleMeshFromNodes);    


    // GeometricalTransformationUtilities
    auto mod_geom_trans_utils = m.def_submodule("GeometricalTransformationUtilities");
    mod_geom_trans_utils.def("CalculateTranslationMatrix", &GeometricalTransformationUtilities::CalculateTranslationMatrix );
    mod_geom_trans_utils.def("CalculateRotationMatrix", &GeometricalTransformationUtilities::CalculateRotationMatrix );

    // GeometricalTransformationUtilities
    auto mod_compare_elem_cond_utils = m.def_submodule("CompareElementsAndConditionsUtility");
    mod_compare_elem_cond_utils.def("GetRegisteredName", GetRegisteredNameElement );
    mod_compare_elem_cond_utils.def("GetRegisteredName", GetRegisteredNameCondition );




    
}

} // namespace Python.
} // Namespace Kratos    
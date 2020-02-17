//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    KratosAppGenerator
//

// System includes
// see the header file

// External includes
// see the header file

// Include Base headers
// see the header file

// Project includes
#include "custom_elements/variational_non_eikonal_distance_element.h"

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
 * Constructor.
 */
VariationalNonEikonalDistanceElement::VariationalNonEikonalDistanceElement(IndexType NewId)
    : Element(NewId) 
{
}

/**
 * Constructor using an array of nodes
 */
VariationalNonEikonalDistanceElement::VariationalNonEikonalDistanceElement(IndexType NewId, const NodesArrayType& ThisNodes)
    : Element(NewId, ThisNodes) 
{
}

/**
 * Constructor using Geometry
 */
VariationalNonEikonalDistanceElement::VariationalNonEikonalDistanceElement(IndexType NewId, GeometryType::Pointer pGeometry)
    : Element(NewId, pGeometry) 
{
}

/**
 * Constructor using Properties
 */
VariationalNonEikonalDistanceElement::VariationalNonEikonalDistanceElement(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties)
    : Element(NewId, pGeometry, pProperties) 
{
}

/**
 * Copy Constructor
 */
VariationalNonEikonalDistanceElement::VariationalNonEikonalDistanceElement(VariationalNonEikonalDistanceElement const& rOther)
    : Element(rOther) 
{
}

/**
 * Destructor
 */
VariationalNonEikonalDistanceElement::~VariationalNonEikonalDistanceElement()
{
}

///@}
///@name Operators
///@{

/// Assignment operator.
VariationalNonEikonalDistanceElement & VariationalNonEikonalDistanceElement::operator=(VariationalNonEikonalDistanceElement const& rOther)
{
    BaseType::operator=(rOther);
    Flags::operator =(rOther);
    // mpProperties = rOther.mpProperties;
    return *this;
}

///@}
///@name Operations
///@{

/**
 * ELEMENTS inherited from this class have to implement next
 * Create and Clone methods: MANDATORY
 */

/**
 * creates a new element pointer
 * @param NewId: the ID of the new element
 * @param ThisNodes: the nodes of the new element
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
Element::Pointer VariationalNonEikonalDistanceElement::Create(
    IndexType NewId,
    NodesArrayType const& ThisNodes,
    PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<VariationalNonEikonalDistanceElement>(NewId, GetGeometry().Create(ThisNodes), pProperties);
    KRATOS_CATCH("");
}

/**
 * creates a new element pointer
 * @param NewId: the ID of the new element
 * @param pGeom: the geometry to be employed
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
Element::Pointer VariationalNonEikonalDistanceElement::Create(
    IndexType NewId,
    GeometryType::Pointer pGeom,
    PropertiesType::Pointer pProperties) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<VariationalNonEikonalDistanceElement>(NewId, pGeom, pProperties);
    KRATOS_CATCH("");
}

/**
 * creates a new element pointer and clones the previous element data
 * @param NewId: the ID of the new element
 * @param ThisNodes: the nodes of the new element
 * @param pProperties: the properties assigned to the new element
 * @return a Pointer to the new element
 */
Element::Pointer VariationalNonEikonalDistanceElement::Clone(IndexType NewId, NodesArrayType const& ThisNodes) const
{
    KRATOS_TRY
    return Kratos::make_intrusive<VariationalNonEikonalDistanceElement>(NewId, GetGeometry().Create(ThisNodes), pGetProperties());
    KRATOS_CATCH("");
}

/**
 * this determines the elemental equation ID vector for all elemental
 * DOFs
 * @param rResult: the elemental equation ID vector
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::EquationIdVector(EquationIdVectorType& rResult, ProcessInfo& CurrentProcessInfo)
{
    //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "EquationIdVector" << std::endl;

    const int num_dim  = 3;
    const int num_nodes  = num_dim + 1;

    // num_dof = 4*num_nodes
    if (rResult.size() != num_nodes){
        rResult.resize(num_nodes, false);
    }

    for(unsigned int i=0; i<num_nodes; i++){
        rResult[i]  =  this->GetGeometry()[i].GetDof(DISTANCE_AUX2).EquationId();
    }
}

/**
 * determines the elemental list of DOFs
 * @param ElementalDofList: the list of DOFs
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::GetDofList(DofsVectorType& rElementalDofList, ProcessInfo& CurrentProcessInfo)
{
    //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "GetDofList" << std::endl;
    
    const int num_dim  = 3;
    const int num_nodes  = num_dim + 1;

    // num_dof = 4*num_nodes
    if (rElementalDofList.size() != num_nodes){
        rElementalDofList.resize(num_nodes);
    }

    for(unsigned int i=0; i<num_nodes; i++){
        rElementalDofList[i] = this->GetGeometry()[i].pGetDof(DISTANCE_AUX2);
    }
}

/**
 * ELEMENTS inherited from this class have to implement next
 * CalculateLocalSystem, CalculateLeftHandSide and CalculateRightHandSide methods
 * they can be managed internally with a private method to do the same calculations
 * only once: MANDATORY
 */

/**
 * this is called during the assembling process in order
 * to calculate all elemental contributions to the global system
 * matrix and the right hand side
 * @param rLeftHandSideMatrix: the elemental left hand side matrix
 * @param rRightHandSideVector: the elemental right hand side
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateLocalSystem(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 1" << std::endl;

    const int num_dim  = 3;
    const int num_nodes  = num_dim + 1;

    const double penalty_phi0 = 1.0e8;

    const double f = 1.0e0;

    GeometryData::ShapeFunctionsGradientsType DN_DX;  
    Matrix N;
    Vector DetJ;
    Vector weights; 

    Vector distances0(num_nodes);
    Vector values(num_nodes);
    Matrix tempPhi = ZeroMatrix(num_nodes,num_nodes);
    Vector rhs = ZeroVector(num_nodes);

    const GeometryData::IntegrationMethod integration_method = GeometryData::GI_GAUSS_2;
    GeometryType::Pointer p_geometry = this->pGetGeometry();
    const unsigned int number_of_gauss_points = p_geometry->IntegrationPointsNumber(integration_method);

    // Getting data for the given geometry
    p_geometry->ShapeFunctionsIntegrationPointsGradients(DN_DX, DetJ, integration_method);

    if (N.size1() != number_of_gauss_points || N.size2() != num_nodes) {
        N.resize(number_of_gauss_points,num_nodes,false);
    }
    N = p_geometry->ShapeFunctionsValues(integration_method);

    const GeometryType::IntegrationPointsArrayType& IntegrationPoints = p_geometry->IntegrationPoints(integration_method);
    if (weights.size() != number_of_gauss_points) {
        weights.resize(number_of_gauss_points,false);
    }
    for (unsigned int gp = 0; gp < number_of_gauss_points; gp++){
        weights[gp] = DetJ[gp] * IntegrationPoints[gp].Weight();
    }

    for (unsigned int i_node=0; i_node < num_nodes; ++i_node){
        distances0(i_node) = (*p_geometry)[i_node].FastGetSolutionStepValue(DISTANCE);
        values(i_node) = (*p_geometry)[i_node].FastGetSolutionStepValue(DISTANCE_AUX2);
    }

    unsigned int nneg=0, npos=0;
    for (unsigned int i_node=0; i_node < num_nodes; ++i_node)
    {
        if (distances0(i_node) > 0.0) npos += 1;
        else /* if (distances0(i_node) < 0.0) */ nneg += 1;
    }

    if(rLeftHandSideMatrix.size1() != num_nodes)
        rLeftHandSideMatrix.resize(num_nodes,num_nodes,false); //resizing the system in case it does not have the right size 

    if(rRightHandSideVector.size() != num_nodes)
        rRightHandSideVector.resize(num_nodes,false);

    //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 2" << std::endl;

    for (unsigned int gp = 0; gp < number_of_gauss_points; gp++){
        for (unsigned int i_node = 0; i_node < num_nodes; i_node++){ 
            for (unsigned int j_node = 0; j_node < num_nodes; j_node++){
                for (unsigned int k_dim = 0; k_dim < num_dim; k_dim++){
                    tempPhi(i_node, j_node) += 
                        weights(gp) * (DN_DX[gp])(i_node, k_dim) * (DN_DX[gp])(j_node, k_dim);
                }
            }
        }
    }

    //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 3" << std::endl;

    if (npos != 0 && nneg != 0)
    {
        ModifiedShapeFunctions::Pointer p_modified_sh_func = 
                Kratos::make_shared<Tetrahedra3D4ModifiedShapeFunctions>(p_geometry, distances0);

        Matrix neg_N, pos_N, int_N;
        GeometryType::ShapeFunctionsGradientsType neg_DN_DX, pos_DN_DX, int_DN_DX;
        Vector neg_weights, pos_weights, int_weights;

        p_modified_sh_func->ComputePositiveSideShapeFunctionsAndGradientsValues(
            pos_N,        // N
            pos_DN_DX,    // DN_DX
            pos_weights,  // weight * detJ
            integration_method);

        const std::size_t number_of_pos_gauss_points = pos_weights.size();

        for (unsigned int pos_gp = 0; pos_gp < number_of_pos_gauss_points; pos_gp++){
            for (unsigned int i_node = 0; i_node < num_nodes; i_node++){ 
                rhs(i_node) += f * pos_weights(pos_gp) * pos_N(pos_gp, i_node);
            }
        }

        p_modified_sh_func->ComputeNegativeSideShapeFunctionsAndGradientsValues(
            neg_N,        // N
            neg_DN_DX,    // DN_DX
            neg_weights,  // weight * detJ
            integration_method);

        const std::size_t number_of_neg_gauss_points = neg_weights.size();

        for (unsigned int neg_gp = 0; neg_gp < number_of_neg_gauss_points; neg_gp++){
            for (unsigned int i_node = 0; i_node < num_nodes; i_node++){ 
                rhs(i_node) -= f * neg_weights(neg_gp) * neg_N(neg_gp, i_node);
            }
        }

        //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 4" << std::endl;

        p_modified_sh_func->ComputeInterfaceNegativeSideShapeFunctionsAndGradientsValues(
            int_N,
            int_DN_DX,
            int_weights,
            integration_method);

        const std::size_t number_of_int_gauss_points = int_weights.size();

        for (unsigned int int_gp = 0; int_gp < number_of_int_gauss_points; int_gp++){
            for (unsigned int i_node = 0; i_node < num_nodes; i_node++){
                for (unsigned int j_node = 0; j_node < num_nodes; j_node++){
                    tempPhi(i_node, j_node) += 
                        penalty_phi0 * int_weights(int_gp) * int_N(int_gp, i_node) * int_N(int_gp, j_node);
                }
            }
        }

        //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 5" << std::endl;

    } else {

        //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 6" << std::endl;
        
        double source;
        if (npos != 0)
            source = f;
        else
            source = -f;

        for (unsigned int gp = 0; gp < number_of_gauss_points; gp++){
            for (unsigned int i_node = 0; i_node < num_nodes; i_node++){ 
                rhs(i_node) += weights(gp) * source * N(gp, i_node);
            }
        } 

        //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 7" << std::endl;          

    }

    noalias(rLeftHandSideMatrix) = tempPhi /* + tempGradPhi */;
    noalias(rRightHandSideVector) = rhs - prod(rLeftHandSideMatrix,values); // Reducing the contribution of the last known values 

    //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Here 8" << std::endl;

    KRATOS_CATCH("");
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental left hand side matrix only
 * @param rLeftHandSideMatrix: the elemental left hand side matrix
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateLeftHandSide(MatrixType& rLeftHandSideMatrix, ProcessInfo& rCurrentProcessInfo)
{
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental right hand side vector only
 * @param rRightHandSideVector: the elemental right hand side vector
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateRightHandSide(VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo)
{
}

/**
 * this is called during the assembling process in order
 * to calculate the first derivatives contributions for the LHS and RHS
 * @param rLeftHandSideMatrix: the elemental left hand side matrix
 * @param rRightHandSideVector: the elemental right hand side
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateFirstDerivativesContributions(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != 0)
        rLeftHandSideMatrix.resize(0, 0, false);
    if (rRightHandSideVector.size() != 0)
        rRightHandSideVector.resize(0, false);
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental left hand side matrix for the first derivatives constributions
 * @param rLeftHandSideMatrix: the elemental left hand side matrix
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateFirstDerivativesLHS(MatrixType& rLeftHandSideMatrix, ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != 0)
        rLeftHandSideMatrix.resize(0, 0, false);
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental right hand side vector for the first derivatives constributions
 * @param rRightHandSideVector: the elemental right hand side vector
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateFirstDerivativesRHS(VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo)
{
    if (rRightHandSideVector.size() != 0)
        rRightHandSideVector.resize(0, false);
}

/**
 * ELEMENTS inherited from this class must implement this methods
 * if they need to add dynamic element contributions
 * note: second derivatives means the accelerations if the displacements are the dof of the analysis
 * note: time integration parameters must be set in the rCurrentProcessInfo before calling these methods
 * CalculateSecondDerivativesContributions,
 * CalculateSecondDerivativesLHS, CalculateSecondDerivativesRHS methods are : OPTIONAL
 */


/**
 * this is called during the assembling process in order
 * to calculate the second derivative contributions for the LHS and RHS
 * @param rLeftHandSideMatrix: the elemental left hand side matrix
 * @param rRightHandSideVector: the elemental right hand side
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateSecondDerivativesContributions(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != 0)
        rLeftHandSideMatrix.resize(0, 0, false);
    if (rRightHandSideVector.size() != 0)
        rRightHandSideVector.resize(0, false);
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental left hand side matrix for the second derivatives constributions
 * @param rLeftHandSideMatrix: the elemental left hand side matrix
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateSecondDerivativesLHS(
    MatrixType& rLeftHandSideMatrix,
    ProcessInfo& rCurrentProcessInfo)
{
    if (rLeftHandSideMatrix.size1() != 0)
        rLeftHandSideMatrix.resize(0, 0, false);
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental right hand side vector for the second derivatives constributions
 * @param rRightHandSideVector: the elemental right hand side vector
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateSecondDerivativesRHS(
    VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo)
{
    if (rRightHandSideVector.size() != 0)
        rRightHandSideVector.resize(0, false);
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental mass matrix
 * @param rMassMatrix: the elemental mass matrix
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateMassMatrix(MatrixType& rMassMatrix, ProcessInfo& rCurrentProcessInfo)
{
    if (rMassMatrix.size1() != 0)
        rMassMatrix.resize(0, 0, false);
}

/**
 * this is called during the assembling process in order
 * to calculate the elemental damping matrix
 * @param rDampingMatrix: the elemental damping matrix
 * @param rCurrentProcessInfo: the current process info instance
 */
void VariationalNonEikonalDistanceElement::CalculateDampingMatrix(MatrixType& rDampingMatrix, ProcessInfo& rCurrentProcessInfo)
{
    if (rDampingMatrix.size1() != 0)
        rDampingMatrix.resize(0, 0, false);
}

/**
 * This method provides the place to perform checks on the completeness of the input
 * and the compatibility with the problem options as well as the contitutive laws selected
 * It is designed to be called only once (or anyway, not often) typically at the beginning
 * of the calculations, so to verify that nothing is missing from the input
 * or that no common error is found.
 * @param rCurrentProcessInfo
 * this method is: MANDATORY
 */
int VariationalNonEikonalDistanceElement::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    //KRATOS_INFO("VariationalNonEikonalDistanceElement") << "Check" << std::endl;

    KRATOS_ERROR_IF(this->Id() < 1) <<"VariationalNonEikonalDistanceElement found with Id 0 or negative" << std::endl;

    KRATOS_ERROR_IF(this->GetGeometry().Area() <= 0) << "On VariationalNonEikonalDistanceElement -> "
        << this->Id() <<  "; Area cannot be less than or equal to 0" << std::endl;
  
      // Base class checks for positive Jacobian and Id > 0
      int ierr = Element::Check(rCurrentProcessInfo);
      if(ierr != 0) return ierr;
  
      // Check that all required variables have been registered
      KRATOS_CHECK_VARIABLE_KEY(DISTANCE)
      KRATOS_CHECK_VARIABLE_KEY(DISTANCE_AUX2)
      KRATOS_CHECK_VARIABLE_KEY(DISTANCE_GRADIENT_X)
      KRATOS_CHECK_VARIABLE_KEY(DISTANCE_GRADIENT_Y)
      KRATOS_CHECK_VARIABLE_KEY(DISTANCE_GRADIENT_Z)
  
      unsigned const int number_of_points = GetGeometry().size(); 
      // Check that the element's nodes contain all required SolutionStepData and Degrees of freedom
      for ( unsigned int i = 0; i < number_of_points; i++ )
      {
          Node<3> &rnode = this->GetGeometry()[i];
          KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISTANCE,rnode)
          KRATOS_CHECK_DOF_IN_NODE(DISTANCE,rnode)
          KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISTANCE_AUX2,rnode)
          KRATOS_CHECK_DOF_IN_NODE(DISTANCE_AUX2,rnode)
          KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISTANCE_GRADIENT_X,rnode)
          KRATOS_CHECK_DOF_IN_NODE(DISTANCE_GRADIENT_X,rnode)
          KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISTANCE_GRADIENT_Y,rnode)
          KRATOS_CHECK_DOF_IN_NODE(DISTANCE_GRADIENT_Y,rnode)
          KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DISTANCE_GRADIENT_Z,rnode)
          KRATOS_CHECK_DOF_IN_NODE(DISTANCE_GRADIENT_Z,rnode)
      }
  
      return ierr;

    KRATOS_CATCH("");
}

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

std::string VariationalNonEikonalDistanceElement::Info() const {
    std::stringstream buffer;
    buffer << "VariationalNonEikonalDistanceElement #" << Id();
    return buffer.str();
}

/// Print information about this object.

void VariationalNonEikonalDistanceElement::PrintInfo(std::ostream& rOStream) const {
    rOStream << "VariationalNonEikonalDistanceElement #" << Id();
}

/// Print object's data.

void VariationalNonEikonalDistanceElement::PrintData(std::ostream& rOStream) const {
    pGetGeometry()->PrintData(rOStream);
}

///@}
///@name Friends
///@{

///@}

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

///@}
///@name Protected  Access
///@{

///@}
///@name Protected Inquiry
///@{

///@}
///@name Protected LifeCycle
///@{

///@}

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
///@name Serialization
///@{

void VariationalNonEikonalDistanceElement::save(Serializer& rSerializer) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element );

    // List
    // To be completed with the class member list
}

void VariationalNonEikonalDistanceElement::load(Serializer& rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element );

    // List
    // To be completed with the class member list
}

///@}
///@name Private  Access
///@{

///@}
///@name Private Inquiry
///@{

///@}
///@name Un accessible methods
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// input stream function
inline std::istream & operator >> (std::istream& rIStream, VariationalNonEikonalDistanceElement& rThis);

/// output stream function
inline std::ostream & operator << (std::ostream& rOStream, const VariationalNonEikonalDistanceElement& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << " : " << std::endl;
    rThis.PrintData(rOStream);
    return rOStream;
}

} // namespace Kratos.
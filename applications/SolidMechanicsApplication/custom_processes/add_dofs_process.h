//
//   Project Name:        KratosSolidMechanicsApplication $
//   Created by:          $Author:            JMCarbonell $
//   Last modified by:    $Co-Author:                     $
//   Date:                $Date:              August 2016 $
//   Revision:            $Revision:                  0.0 $
//
//

#if !defined(KRATOS_ADD_DOFS_PROCESS_H_INCLUDED )
#define  KRATOS_ADD_DOFS_PROCESS_H_INCLUDED



// System includes

// External includes

// Project includes
#include "includes/model_part.h"
#include "includes/kratos_parameters.h"
#include "processes/process.h"

namespace Kratos
{

///@name Kratos Classes
///@{

/// The base class for fixing scalar variable Dof or array_1d component Dof processes in Kratos.
/** This function fix the variable dof belonging to all of the nodes in a given mesh
*/
class AddDofsProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    typedef Variable<array_1d<double, 3> >                                    VectorVariableType;
    typedef Variable<double>                                                  ScalarVariableType;
    typedef VariableComponent< VectorComponentAdaptor<array_1d<double, 3> > >      ComponentType;
  
    /// Pointer definition of AddDofsProcess
    KRATOS_CLASS_POINTER_DEFINITION(AddDofsProcess);

    ///@}
    ///@name Life Cycle
    ///@{
    AddDofsProcess(ModelPart& model_part,
		   Parameters rParameters
		   ) : Process() , mr_model_part(model_part)
    {
        KRATOS_TRY
	  
        Parameters default_parameters( R"(
            {
                "model_part_name":"PLEASE_CHOOSE_MODEL_PART_NAME",
                "variables_list": [],
                "reactions_list": []

            }  )" );


        // Validate against defaults -- this ensures no type mismatch
        rParameters.ValidateAndAssignDefaults(default_parameters);

	// Check variables vs reactions consistency
	if( rParameters["variables_list"].size() != rParameters["reactions_list"].size() )
	  KRATOS_ERROR << "variables_list and reactions_list has not the same number of components "<<std::endl;
	
	
	for(unsigned int i=0; i<rParameters["variables_list"].size(); i++)
	  {
	    if( !rParameters["variables_list"][i].IsString() )
	      KRATOS_ERROR << "variables_list contains a non-string variable name "<<std::endl;
	    
	    std::string variable_name = rParameters["variables_list"][i].GetString();

	    bool supplied_reaction = true;
	    if(rParameters["reactions_list"][i].IsNull())
	      supplied_reaction = false;
	    
	    if( KratosComponents< VectorVariableType >::Has( variable_name ) ){ //case of array_1d (vector with components) variable

	      const VectorVariableType& VectorVariable = KratosComponents< VectorVariableType >::Get(variable_name);
	      if( model_part.GetNodalSolutionStepVariablesList().Has( VectorVariable ) == false ){
		KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	      }
	      else{
		for(unsigned int j=0; j<3; j++)
		  {
		    std::string component_name = variable_name;
		    component_name += ms_components[j];		  
		    const ComponentType& ComponentVariable = KratosComponents< ComponentType >::Get(component_name);
		    		    
		    if(supplied_reaction){		      	      
		      std::string reaction_component_name = rParameters["reactions_list"][i].GetString();
		      reaction_component_name += ms_components[j];		  
		      const ComponentType& ReactionComponentVariable = KratosComponents< ComponentType >::Get(reaction_component_name);
		      m_component_variables_list.push_back(&ComponentVariable);	    
		      m_component_reactions_list.push_back(&ReactionComponentVariable);
		    }
		    else{
		      m_component_variables_no_reaction_list.push_back(&ComponentVariable);
		    }

		  }
	      }
	    }
	    else if( KratosComponents< ComponentType >::Has(variable_name) ){ //case of component variable
	      
	      const ComponentType& ComponentVariable = KratosComponents< ComponentType >::Get(variable_name);
		
	      if( model_part.GetNodalSolutionStepVariablesList().Has( ComponentVariable.GetSourceVariable() ) == false ){
		  
		KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	      }
	      else{
		  		  
		if(supplied_reaction){
		  std::string reaction_name = rParameters["reactions_list"][i].GetString();
		  const ComponentType& ReactionComponentVariable = KratosComponents< ComponentType >::Get(reaction_name);
		  m_component_variables_list.push_back(&ComponentVariable);
		  m_component_reactions_list.push_back(&ReactionComponentVariable);
		}
		else{
		  m_component_variables_no_reaction_list.push_back(&ComponentVariable);
		}
					      
	      }
		  
	  
	    }
	    else if( KratosComponents< ScalarVariableType >::Has( variable_name ) ){ //case of double variable
	      
	      const ScalarVariableType& ScalarVariable = KratosComponents< ScalarVariableType >::Get( variable_name );
	      if( model_part.GetNodalSolutionStepVariablesList().Has( ScalarVariable ) ==  false ){
		KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	      }
	      else{
		  
		if(supplied_reaction){
		  std::string reaction_name = rParameters["reactions_list"][i].GetString();
		  const ScalarVariableType& ReactionVariable = KratosComponents< ScalarVariableType >::Get(reaction_name);
		  m_scalar_variables_list.push_back(&ScalarVariable);
		  m_scalar_reactions_list.push_back(&ReactionVariable);
		}
		else{
		  m_scalar_variables_no_reaction_list.push_back(&ScalarVariable);
		}

	      }
			      
	    }
	    else{
	      KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	    }
	  }

	       
        KRATOS_CATCH("")
    }

    
    AddDofsProcess(ModelPart& model_part,
		   const boost::python::list& rVariablesList,
		   const boost::python::list& rReactionsList
		   ) : Process(), mr_model_part(model_part)
    {
        KRATOS_TRY
	  
	unsigned int number_variables = len(rVariablesList);
	unsigned int number_reactions = len(rVariablesList);

	// Check variables vs reactions consistency
	if( number_variables != number_reactions )
	  KRATOS_ERROR << "variables_list and reactions_list has not the same number of components "<<std::endl;

	for(unsigned int i=0; i<number_variables; i++)
	  {

	    std::string variable_name = boost::python::extract<std::string>(rVariablesList[i]);
	    std::string reaction_name = boost::python::extract<std::string>(rReactionsList[i]);

	    bool supplied_reaction = true;
	    if(reaction_name == "NONE")
	      supplied_reaction = false;
	    
	    if( KratosComponents< VectorVariableType >::Has( variable_name ) ){ //case of array_1d (vector with components) variable

	      const VectorVariableType& VectorVariable = KratosComponents< VectorVariableType >::Get(variable_name);
	      if( model_part.GetNodalSolutionStepVariablesList().Has( VectorVariable ) == false ){
		KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	      }
	      else{
		for(unsigned int j=0; j<3; j++)
		  {
		    std::string component_name = variable_name;
		    component_name += ms_components[j];		  
		    const ComponentType& ComponentVariable = KratosComponents< ComponentType >::Get(component_name);
		    		    
		    if(supplied_reaction){		      	      
		      std::string reaction_component_name = reaction_name;
		      reaction_component_name += ms_components[j];		  
		      const ComponentType& ReactionComponentVariable = KratosComponents< ComponentType >::Get(reaction_component_name);
		      m_component_variables_list.push_back(&ComponentVariable);
		      m_component_reactions_list.push_back(&ReactionComponentVariable); 		      
		    }
		    else{
		      m_component_variables_no_reaction_list.push_back(&ComponentVariable);
		    }

		  }
	      }
	    }
	    else if( KratosComponents< ComponentType >::Has(variable_name) ){ //case of component variable
	      
	      const ComponentType& ComponentVariable = KratosComponents< ComponentType >::Get(variable_name);
		
	      if( model_part.GetNodalSolutionStepVariablesList().Has( ComponentVariable.GetSourceVariable() ) == false ){
		  
		KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	      }
	      else{
		  		  
		if(supplied_reaction){
		  const ComponentType& ReactionComponentVariable = KratosComponents< ComponentType >::Get(reaction_name);
		  m_component_variables_list.push_back(&ComponentVariable);
		  m_component_reactions_list.push_back(&ReactionComponentVariable);
		}
		else{
		  m_component_variables_list.push_back(&ComponentVariable);
		}
					      
	      }
		  	  
	    }
	    else if( KratosComponents< ScalarVariableType >::Has( variable_name ) ){ //case of double variable
	      
	      const ScalarVariableType& ScalarVariable = KratosComponents< ScalarVariableType >::Get( variable_name );
	      if( model_part.GetNodalSolutionStepVariablesList().Has( ScalarVariable ) ==  false ){
		KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	      }
	      else{
		  
		if(supplied_reaction){
		  const ScalarVariableType& ReactionVariable = KratosComponents< ScalarVariableType >::Get(reaction_name);
		  m_scalar_variables_list.push_back(&ScalarVariable);
		  m_scalar_reactions_list.push_back(&ReactionVariable);
		}
		else{
		  m_scalar_variables_no_reaction_list.push_back(&ScalarVariable);
		}

	      }
			      
	    }
	    else{	     
	      KRATOS_ERROR << "trying to set a variable that is not in the model_part - variable name is "<<variable_name<<std::endl;
	    }
	  }

        KRATOS_CATCH("")
    }


    /// Destructor.
    virtual ~AddDofsProcess() {}


    ///@}
    ///@name Operators
    ///@{

    /// This operator is provided to call the process as a function and simply calls the Execute method.
    void operator()()
    {
        Execute();
    }


    ///@}
    ///@name Operations
    ///@{


    /// Execute method is used to execute the AddDofsProcess algorithms.
    virtual void Execute() 
    {

        KRATOS_TRY;

	int number_of_nodes = mr_model_part.NumberOfNodes();
	ModelPart::NodeConstantIterator nodes_begin = mr_model_part.NodesBegin();

	
	//generating the dofs for the initial node
	AddNodalDofs(nodes_begin);
	
	ModelPart::NodeType::DofsContainerType& reference_dofs = nodes_begin->GetDofs();

        #pragma omp parallel for
	for (int k=0; k<number_of_nodes; k++)
	  {
	    ModelPart::NodeConstantIterator it = nodes_begin + k;

	    for(ModelPart::NodeType::DofsContainerType::iterator iii = reference_dofs.begin(); iii != reference_dofs.end(); iii++)
	      {
		it->pAddDof( *iii );
	      }
	  }
		
	/* this is slower
        #pragma omp parallel for
	for (int k=0; k<number_of_nodes; k++)
	  {
	    ModelPart::NodeConstantIterator it = nodes_begin + k;
	    AddNodalDofs(it);
	  }
	*/
	
        KRATOS_CATCH("");

    }

    /// this function is designed for being called at the beginning of the computations
    /// right after reading the model and the groups
    virtual void ExecuteInitialize()
    {
    }

    /// this function is designed for being execute once before the solution loop but after all of the
    /// solvers where built
    virtual void ExecuteBeforeSolutionLoop()
    {
    }


    /// this function will be executed at every time step BEFORE performing the solve phase
    virtual void ExecuteInitializeSolutionStep()
    {
    }

    /// this function will be executed at every time step AFTER performing the solve phase
    virtual void ExecuteFinalizeSolutionStep()
    {
    }


    /// this function will be executed at every time step BEFORE  writing the output
    virtual void ExecuteBeforeOutputStep()
    {
    }


    /// this function will be executed at every time step AFTER writing the output
    virtual void ExecuteAfterOutputStep()
    {
    }


    /// this function is designed for being called at the end of the computations
    /// right after reading the model and the groups
    virtual void ExecuteFinalize()
    {
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
    virtual std::string Info() const
    {
        return "AddDofsProcess";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const
    {
        rOStream << "AddDofsProcess";
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const
    {
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

    /// Copy constructor.
    AddDofsProcess(AddDofsProcess const& rOther);

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

private:

    ///@name Static Member Variables
    ///@{
    ///@}
    ///@name Member Variables
    ///@{

    ModelPart& mr_model_part;

    const std::vector<std::string> ms_components {"_X", "_Y", "_Z"};
    
    std::vector<ComponentType const *> m_component_variables_list;
    std::vector<ComponentType const *> m_component_reactions_list;
    std::vector<ComponentType const *> m_component_variables_no_reaction_list;
   
    std::vector<ScalarVariableType const *> m_scalar_variables_list;
    std::vector<ScalarVariableType const *> m_scalar_reactions_list;
    std::vector<ScalarVariableType const *> m_scalar_variables_no_reaction_list;


    ///@}
    ///@name Private Operators
    ///@{

    void AddNodalDofs( ModelPart::NodeConstantIterator& node_it )
    {
      KRATOS_TRY

      for( unsigned int i=0; i < m_component_variables_list.size(); i++ )
	{
	  node_it->AddDof(*m_component_variables_list[i],*m_component_reactions_list[i]);
	}
      
      for( unsigned int j=0; j < m_component_variables_no_reaction_list.size(); j++ )
	{
	  node_it->AddDof(*m_component_variables_no_reaction_list[j]);
	}
      
      for( unsigned int l=0; l < m_scalar_variables_list.size(); l++ )
	{
	  node_it->AddDof(*m_scalar_variables_list[l],*m_scalar_reactions_list[l]);
	}
      
      for( unsigned int m=0; m < m_scalar_variables_no_reaction_list.size(); m++ )
	{
	  node_it->AddDof(*m_scalar_variables_no_reaction_list[m]);
	}

      KRATOS_CATCH(" ")
    }
    
    ///@}
    ///@name Private Operations
    ///@{
    ///@}
    ///@name Private  Access
    ///@{

    /// Assignment operator.
    AddDofsProcess& operator=(AddDofsProcess const& rOther);


    ///@}
    ///@name Serialization
    ///@{
    ///@name Private Inquiry
    ///@{
    ///@}
    ///@name Un accessible methods
    ///@{
    ///@}

}; // Class AddDofsProcess

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  AddDofsProcess& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const AddDofsProcess& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}


}  // namespace Kratos.

#endif // KRATOS_ADD_DOFS_PROCESS_H_INCLUDED  defined

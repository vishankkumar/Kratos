//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:        BSD License
//                  Kratos default license: kratos/license.txt
//
//  Main authors:   Riccardo Rossi
//                  Raul Bravo                        
//
//
#if !defined(KRATOS_HROM_BUILDER_AND_SOLVER )
#define  KRATOS_HROM_BUILDER_AND_SOLVER

/* System includes */

/* External includes */

/* Project includes */
#include "includes/define.h"
#include "includes/model_part.h"
#include "solving_strategies/schemes/scheme.h"
#include "solving_strategies/builder_and_solvers/builder_and_solver.h"

/* Application includes */
#include "rom_application_variables.h"
//#include "custom_strategies/rom_builder_and_solver.h"

namespace Kratos
{

template<class TSparseSpace,
         class TDenseSpace, // = DenseSpace<double>,
         class TLinearSolver //= LinearSolver<TSparseSpace,TDenseSpace>
         >
class HROMBuilderAndSolver : public BuilderAndSolver<TSparseSpace,TDenseSpace,TLinearSolver>
{
public:

    /**
     * This struct is used in the component wise calculation only
     * is defined here and is used to declare a member variable in the component wise builder and solver
     * private pointers can only be accessed by means of set and get functions
     * this allows to set and not copy the Element_Variables and Condition_Variables
     * which will be asked and set by another strategy object
     */

    //pointer definition

    KRATOS_CLASS_POINTER_DEFINITION(HROMBuilderAndSolver);

    // The size_t types
    typedef std::size_t SizeType;
    typedef std::size_t IndexType;

    /// Definition of the classes from the base class
    typedef BuilderAndSolver<TSparseSpace,TDenseSpace,TLinearSolver> BaseType;
    typedef typename BaseType::TSchemeType TSchemeType;
    typedef typename BaseType::TDataType TDataType;
    typedef typename BaseType::DofsArrayType DofsArrayType;
    typedef typename BaseType::TSystemMatrixType TSystemMatrixType;
    typedef typename BaseType::TSystemVectorType TSystemVectorType;
    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;
    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;
    typedef typename BaseType::TSystemMatrixPointerType TSystemMatrixPointerType;
    typedef typename BaseType::TSystemVectorPointerType TSystemVectorPointerType;
    typedef typename BaseType::NodesArrayType NodesArrayType;
    typedef typename BaseType::ElementsArrayType ElementsArrayType;
    typedef typename BaseType::ConditionsArrayType ConditionsArrayType;

    /// Additional definitions
    typedef PointerVectorSet<Element, IndexedObject> ElementsContainerType;
    typedef Element::EquationIdVectorType EquationIdVectorType;
    typedef Element::DofsVectorType DofsVectorType;
    typedef boost::numeric::ublas::compressed_matrix<double> CompressedMatrixType;

    /// DoF types definition
    typedef Node<3> NodeType;
    typedef typename NodeType::DofType DofType;
    typedef typename DofType::Pointer DofPointerType;


    /*@} */
    /**@name Life Cycle
     */
    /*@{ */

    /**
     * @brief Default constructor. (with parameters)
     */
    explicit HROMBuilderAndSolver(typename TLinearSolver::Pointer pNewLinearSystemSolver, Parameters ThisParameters)
    : BuilderAndSolver<TSparseSpace,TDenseSpace,TLinearSolver>(pNewLinearSystemSolver)
    {
        // Validate default parameters
        Parameters default_parameters = Parameters(R"(
        {
            "nodal_unknowns" : [],
            "number_of_rom_dofs" : 10
        })" );

        ThisParameters.ValidateAndAssignDefaults(default_parameters);

        // We set the other member variables
        mpLinearSystemSolver = pNewLinearSystemSolver;

        mNodalVariablesNames = ThisParameters["nodal_unknowns"].GetStringArray();
        
        mNodalDofs = mNodalVariablesNames.size();
        mRomDofs = ThisParameters["number_of_rom_dofs"].GetInt();
    }



    /** Destructor.
     */
	~HROMBuilderAndSolver() = default;


    virtual void SetUpDofSet(
		typename TSchemeType::Pointer pScheme,
		ModelPart& rModelPart
	) override
	{
		KRATOS_TRY;

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)) << "Setting up the dofs" << std::endl;

		//Gets the array of elements from the modeler
		auto& r_elements_array = rModelPart.Elements();
		const int number_of_elements = static_cast<int>(r_elements_array.size());

		DofsVectorType dof_list, second_dof_list; // NOTE: The second dof list is only used on constraints to include master/slave relations

		unsigned int nthreads = OpenMPUtils::GetNumThreads();

		typedef std::unordered_set < NodeType::DofType::Pointer, DofPointerHasher>  set_type;

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 2)) << "Number of threads" << nthreads << "\n" << std::endl;

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 2)) << "Initializing element loop" << std::endl;

		/**
		 * Here we declare three sets.
		 * - The global set: Contains all the DoF of the system
		 * - The slave set: The DoF that are not going to be solved, due to MPC formulation
		 */
		set_type dof_global_set;
		dof_global_set.reserve(number_of_elements * 20);

#pragma omp parallel firstprivate(dof_list, second_dof_list)
		{
			auto& r_current_process_info = rModelPart.GetProcessInfo();

			// We cleate the temporal set and we reserve some space on them
			set_type dofs_tmp_set;
			dofs_tmp_set.reserve(20000);

			// Gets the array of elements from the modeler
#pragma omp for schedule(guided, 512) nowait
			for (int i = 0; i < number_of_elements; ++i) {
				auto it_elem = r_elements_array.begin() + i;

				// Gets list of Dof involved on every element
				pScheme->GetElementalDofList(*(it_elem.base()), dof_list, r_current_process_info);
				dofs_tmp_set.insert(dof_list.begin(), dof_list.end());
			}

			// Gets the array of conditions from the modeler
			ConditionsArrayType& r_conditions_array = rModelPart.Conditions();
			const int number_of_conditions = static_cast<int>(r_conditions_array.size());
#pragma omp for  schedule(guided, 512) nowait
			for (int i = 0; i < number_of_conditions; ++i) {
				auto it_cond = r_conditions_array.begin() + i;

				// Gets list of Dof involved on every element
				pScheme->GetConditionDofList(*(it_cond.base()), dof_list, r_current_process_info);
				dofs_tmp_set.insert(dof_list.begin(), dof_list.end());
			}

			// Gets the array of constraints from the modeler
			auto& r_constraints_array = rModelPart.MasterSlaveConstraints();
			const int number_of_constraints = static_cast<int>(r_constraints_array.size());
#pragma omp for  schedule(guided, 512) nowait
			for (int i = 0; i < number_of_constraints; ++i) {
				auto it_const = r_constraints_array.begin() + i;

				// Gets list of Dof involved on every element
				it_const->GetDofList(dof_list, second_dof_list, r_current_process_info);
				dofs_tmp_set.insert(dof_list.begin(), dof_list.end());
				dofs_tmp_set.insert(second_dof_list.begin(), second_dof_list.end());
			}

			// We merge all the sets in one thread
#pragma omp critical
			{
				dof_global_set.insert(dofs_tmp_set.begin(), dofs_tmp_set.end());
			}
		}

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 2)) << "Initializing ordered array filling\n" << std::endl;

        DofsArrayType Doftemp;
        BaseType::mDofSet = DofsArrayType();

        Doftemp.reserve(dof_global_set.size());
        for (auto it = dof_global_set.begin(); it != dof_global_set.end(); it++)
        {
            Doftemp.push_back(*it);
        }
        Doftemp.Sort();

		BaseType::mDofSet = Doftemp;

		//Throws an exception if there are no Degrees Of Freedom involved in the analysis
		KRATOS_ERROR_IF(BaseType::mDofSet.size() == 0) << "No degrees of freedom!" << std::endl;

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 2)) << "Number of degrees of freedom:" << BaseType::mDofSet.size() << std::endl;

		BaseType::mDofSetIsInitialized = true;

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 2 && rModelPart.GetCommunicator().MyPID() == 0)) << "Finished setting up the dofs" << std::endl;

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 2)) << "End of setup dof set\n" << std::endl;


#ifdef KRATOS_DEBUG
		// If reactions are to be calculated, we check if all the dofs have reactions defined
		// This is tobe done only in debug mode
		if (BaseType::GetCalculateReactionsFlag()) {
			for (auto dof_iterator = BaseType::mDofSet.begin(); dof_iterator != BaseType::mDofSet.end(); ++dof_iterator) {
				KRATOS_ERROR_IF_NOT(dof_iterator->HasReaction()) << "Reaction variable not set for the following : " << std::endl
					<< "Node : " << dof_iterator->Id() << std::endl
					<< "Dof : " << (*dof_iterator) << std::endl << "Not possible to calculate reactions." << std::endl;
			}
		}
#endif
		// {
		// 	mDofList.clear();
		// 	for (auto& node : rModelPart.Nodes())
		// 	{
		// 		for (const std::string& var_name : mNodalVariablesNames)
		// 		{
		// 			if (KratosComponents< Variable<double> >::Has(var_name)) //case of double variable
		// 			{
		// 				auto pdof = node.pGetDof(KratosComponents< Variable<double> >::Get(var_name));
		// 				mDofList.push_back(pdof);
		// 			}
		// 			else if (KratosComponents< VariableComponent< VectorComponentAdaptor<array_1d<double, 3> > > >::Has(var_name)) //case of component variable
		// 			{
		// 				typedef VariableComponent< VectorComponentAdaptor<array_1d<double, 3> > > component_type;
		// 				component_type var_component = KratosComponents< component_type >::Get(var_name);
		// 				auto pdof = node.pGetDof(var_component.GetSourceVariable());
		// 				mDofList.push_back(pdof);
		// 			}
		// 		}
		// 	}
		// }


		KRATOS_CATCH("");
	}

    /**
            organises the dofset in order to speed up the building phase
     */
    virtual void SetUpSystem(ModelPart& r_model_part) override
    {
		//int free_id = 0;
		BaseType::mEquationSystemSize = BaseType::mDofSet.size();
		int ndofs = static_cast<int>(BaseType::mDofSet.size());

#pragma omp parallel for firstprivate(ndofs)
		for (int i = 0; i < static_cast<int>(ndofs); i++) {
			typename DofsArrayType::iterator dof_iterator = BaseType::mDofSet.begin() + i;
			dof_iterator->SetEquationId(i);

		}
    }

    Vector ProjectToReducedBasis(
		const TSystemVectorType& rX,
		ModelPart::NodesContainerType& rNodes
	)
    {
        Vector rom_unknowns = ZeroVector(mRomDofs);
        for(const auto& node : rNodes)
        {
            unsigned int node_aux_id = node.GetValue(AUX_ID);
            const auto& nodal_rom_basis = node.GetValue(ROM_BASIS);
				for (int i = 0; i < mRomDofs; ++i) {
					for (int j = 0; j < mNodalDofs; ++j) {
						rom_unknowns[i] += nodal_rom_basis(j, i)*rX(node_aux_id*mNodalDofs + j);
					}
				}
        }
        return rom_unknowns;
	}

    void ProjectToFineBasis(
	    const TSystemVectorType& rRomUnkowns,
		ModelPart::NodesContainerType& rNodes,
		TSystemVectorType& rX
	)
    {
        TSparseSpace::SetToZero(rX);
        for(const auto& node : rNodes)
        {
            unsigned int node_aux_id = node.GetValue(AUX_ID);
            const auto& nodal_rom_basis = node.GetValue(ROM_BASIS);
            Vector tmp = prod(nodal_rom_basis, rRomUnkowns );
			for (unsigned int i = 0; i < tmp.size(); ++i)
			{
			rX[node_aux_id*mNodalDofs + i] = tmp[i];
			}
        }
    }



    // void GetDofValues(const std::vector<DofPointerType>& rDofList, TSystemVectorType& rX)
    // {
    //     unsigned int i=0;
    //     for(auto& dof : rDofList)
    //         rX[i++] = dof->GetSolutionStepValue();
    // }

    /*@{ */

    /**
            This should be added in a separate utility, callable from Python at the end of the simulation
     */
    virtual void BuildAndSolve(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b) override
	{
        //define a dense matrix to hold the reduced problem
        Matrix Arom = ZeroMatrix(mRomDofs,mRomDofs);
        Vector brom = ZeroVector(mRomDofs);
        TSystemVectorType x(Dx.size());
        

        // //find the rom basis
        // this->GetDofValues(mDofList,x);

		double start_build4 = OpenMPUtils::GetCurrentTime();
		Vector xrom = this->ProjectToReducedBasis(x, rModelPart.Nodes());
		const double stop_build4 = OpenMPUtils::GetCurrentTime();
		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() >= 1 && rModelPart.GetCommunicator().MyPID() == 0)) << "Project to reduced basis time: " << stop_build4 - start_build4 << std::endl;


        //build the system matrix by looping over elements and conditions and assembling to A
        KRATOS_ERROR_IF(!pScheme) << "No scheme provided!" << std::endl;

        // Getting the elements from the model
        const int nelements = static_cast<int>(rModelPart.Elements().size());

        // Getting the array of the conditions
        const int nconditions = static_cast<int>(rModelPart.Conditions().size());

        auto& CurrentProcessInfo = rModelPart.GetProcessInfo();
        auto el_begin = rModelPart.ElementsBegin();
        auto cond_begin = rModelPart.ConditionsBegin();

        //contributions to the system
        LocalSystemMatrixType LHS_Contribution = LocalSystemMatrixType(0, 0);
        LocalSystemVectorType RHS_Contribution = LocalSystemVectorType(0);

        //vector containing the localization in the system of the different
        //terms
        Element::EquationIdVectorType EquationId;

        // assemble all elements
        double start_build = OpenMPUtils::GetCurrentTime();

        #pragma omp parallel firstprivate(nelements,nconditions, LHS_Contribution, RHS_Contribution, EquationId)
        {
            Matrix tempA = ZeroMatrix(mRomDofs,mRomDofs);
            Vector tempb = ZeroVector(mRomDofs);
            #pragma omp for nowait
            for (int k = 0; k < nelements; k++)
            {
                auto it_el = el_begin + k;
                //detect if the element is active or not. If the user did not make any choice the element
                //is active by default
                bool element_is_active = false;
                if ((it_el)->Has(HROM_WEIGHT))
                    //KRATOS_WATCH(k);
                    element_is_active = true;            
                
                if (element_is_active)
                {
                    //calculate elemental contribution
                    pScheme->CalculateSystemContributions(*(it_el.base()), LHS_Contribution, RHS_Contribution, EquationId, CurrentProcessInfo);                    
                    Element::DofsVectorType dofs;
                    it_el->GetDofList(dofs, CurrentProcessInfo);

                    //assemble the elemental contribution - here is where the ROM acts
                    //compute the elemental reduction matrix T
                    const auto& geom = it_el->GetGeometry();
                    Matrix Telemental(geom.size()*mNodalDofs, mRomDofs);

                    for(unsigned int i=0; i<geom.size(); ++i)
                    {
                        const Matrix& rom_nodal_basis = geom[i].GetValue(ROM_BASIS);
                        for(unsigned int k=0; k<rom_nodal_basis.size1(); ++k)
                        {
                            if (dofs[i*mNodalDofs + k]->IsFixed())
                                row(Telemental, i*mNodalDofs + k) = ZeroVector(Telemental.size2());
                            else
                                row(Telemental, i*mNodalDofs+k) = row(rom_nodal_basis,k);
                        }
                    }
                    double H_ROM_WEIGHT = it_el->GetValue(HROM_WEIGHT);
                    Matrix aux = prod(LHS_Contribution, Telemental);
                    
                    noalias(tempA) += prod(trans(Telemental), aux) * H_ROM_WEIGHT;
                    noalias(tempb) += prod(trans(Telemental), RHS_Contribution) * H_ROM_WEIGHT;

                    // clean local elemental me overridemory
                    pScheme->CleanMemory(*(it_el.base()));                
                }
                
            }
            
            #pragma omp for
            for (int k = 0; k < nconditions;  k++)
            {
                ModelPart::ConditionsContainerType::iterator it = cond_begin + k;

                //detect if the element is active or not. If the user did not make any choice the element
                //is active by default
                bool condition_is_active = false;
                if ((it)->Has(HROM_WEIGHT))
                    //KRATOS_WATCH(k);
                    condition_is_active = true;

                if (condition_is_active)
                {
                    Condition::DofsVectorType dofs;
                    it->GetDofList(dofs, CurrentProcessInfo);
                    //calculate elemental contribution
                    pScheme->Condition_CalculateSystemContributions(*(it.base()), LHS_Contribution, RHS_Contribution, EquationId, CurrentProcessInfo);

                    //assemble the elemental contribution - here is where the ROM acts
                    //compute the elemental reduction matrix T

                    const auto& r_geom = it->GetGeometry();
                    Matrix Telemental(r_geom.size()*mNodalDofs, mRomDofs);

                    for(unsigned int i=0; i<r_geom.size(); ++i)
                    {
                        const Matrix& rom_nodal_basis = r_geom[i].GetValue(ROM_BASIS);
                        for(unsigned int k=0; k<rom_nodal_basis.size1(); ++k)
                        {
                            if (dofs[i*mNodalDofs + k]->IsFixed())
                                row(Telemental, i*mNodalDofs + k) = ZeroVector(Telemental.size2());
                            else
                                row(Telemental, i*mNodalDofs+k) = row(rom_nodal_basis,k);
                        }
                    }
                    double H_ROM_WEIGHT = it->GetValue(HROM_WEIGHT);
                    Matrix aux = prod(LHS_Contribution, Telemental);
                    
                    noalias(tempA) += prod(trans(Telemental), aux) * H_ROM_WEIGHT;
                    noalias(tempb) += prod(trans(Telemental), RHS_Contribution) * H_ROM_WEIGHT;
                    

                    // clean local elemental memory
                    pScheme->CleanMemory(*(it.base()));
                }
            }
            #pragma omp critical 
            {
                noalias(Arom) +=tempA;
                noalias(brom) +=tempb;
            }            

        }
        // Checking the already fully built matrices 

        //KRATOS_WATCH(Arom);
        //KRATOS_WATCH(brom);



        const double stop_build = OpenMPUtils::GetCurrentTime();
        KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() >= 1 && rModelPart.GetCommunicator().MyPID() == 0)) << "Build time: " << stop_build - start_build << std::endl;

        KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() > 2 && rModelPart.GetCommunicator().MyPID() == 0)) << "Finished parallel building" << std::endl;

        //solve for the rom unkowns dunk = Arom^-1 * brom
        Vector dxrom(xrom.size());
		double start_build2 = OpenMPUtils::GetCurrentTime();
		MathUtils<double>::Solve(Arom, dxrom, brom);
		const double stop_build2 = OpenMPUtils::GetCurrentTime();

		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() >= 1 && rModelPart.GetCommunicator().MyPID() == 0)) << "Solve reduced system time: " << stop_build2 - start_build2 << std::endl;

		//update database
		noalias(xrom) += dxrom;

		double start_build3 = OpenMPUtils::GetCurrentTime();
		ProjectToFineBasis(dxrom, rModelPart.Nodes(), Dx);
		const double stop_build3 = OpenMPUtils::GetCurrentTime();
		KRATOS_INFO_IF("HROMBuilderAndSolver", (this->GetEchoLevel() >= 1 && rModelPart.GetCommunicator().MyPID() == 0)) << "Project to fine basis time: " << stop_build3 - start_build3 << std::endl;

    }

	void ResizeAndInitializeVectors(
		typename TSchemeType::Pointer pScheme,
		TSystemMatrixPointerType& pA,
		TSystemVectorPointerType& pDx,
		TSystemVectorPointerType& pb,
		ModelPart& rModelPart
	) override
	{
			KRATOS_TRY
			if (pA == NULL) //if the pointer is not initialized initialize it to an empty matrix
			{
				TSystemMatrixPointerType pNewA = TSystemMatrixPointerType(new TSystemMatrixType(0, 0));
				pA.swap(pNewA);
			}
		if (pDx == NULL) //if the pointer is not initialized initialize it to an empty matrix
		{
			TSystemVectorPointerType pNewDx = TSystemVectorPointerType(new TSystemVectorType(0));
			pDx.swap(pNewDx);
		}
		if (pb == NULL) //if the pointer is not initialized initialize it to an empty matrix
		{
			TSystemVectorPointerType pNewb = TSystemVectorPointerType(new TSystemVectorType(0));
			pb.swap(pNewb);
		}

		TSystemMatrixType& A = *pA;
		TSystemVectorType& Dx = *pDx;
		TSystemVectorType& b = *pb;

		if (Dx.size() != BaseType::mEquationSystemSize)
			Dx.resize(BaseType::mEquationSystemSize, false);
		if (b.size() != BaseType::mEquationSystemSize)
			b.resize(BaseType::mEquationSystemSize, false);

		KRATOS_CATCH("")
    }




    /*@} */
    /**@name Operations */
    /*@{ */


    /*@} */
    /**@name Access */
    /*@{ */


    /*@} */
    /**@name Inquiry */
    /*@{ */

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const override
    {
        return "HROMBuilderAndSolver";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const override
    {
        rOStream << Info();
    }

    /*@} */
    /**@name Friends */
    /*@{ */


    /*@} */

protected:
    /**@name Protected static Member Variables */
    /*@{ */


    /*@} */
    /**@name Protected member Variables */
    /*@{ */

    /** Pointer to the Model.
     */
    typename TLinearSolver::Pointer mpLinearSystemSolver;

    DofsArrayType mDofSet;
    // std::vector<DofPointerType> mDofList;

    bool mReshapeMatrixFlag = false;

    /// flag taking care if the dof set was initialized ot not
    bool mDofSetIsInitialized = false;

    /// flag taking in account if it is needed or not to calculate the reactions
    bool mCalculateReactionsFlag = false;

    /// number of degrees of freedom of the problem to be solve
    unsigned int mEquationSystemSize;
    /*@} */
    /**@name Protected Operators*/
    /*@{ */

    int mEchoLevel = 0;

    TSystemVectorPointerType mpReactionsVector;

    std::vector< std::string > mNodalVariablesNames;
    int mNodalDofs;
    int mRomDofs;



    /*@} */
    /**@name Protected Operations*/
    /*@{ */


    /*@} */
    /**@name Protected  Access */
    /*@{ */


    /*@} */
    /**@name Protected Inquiry */
    /*@{ */


    /*@} */
    /**@name Protected LifeCycle */
    /*@{ */



    /*@} */

private:
    /**@name Static Member Variables */
    /*@{ */


    /*@} */
    /**@name Member Variables */
    /*@{ */

    /*@} */
    /**@name Private Operators*/
    /*@{ */


    /*@} */
    /**@name Private Operations*/
    /*@{ */


    /*@} */
    /**@name Private  Access */
    /*@{ */


    /*@} */
    /**@name Private Inquiry */
    /*@{ */


    /*@} */
    /**@name Un accessible methods */
    /*@{ */


    /*@} */

}; /* Class HROMBuilderAndSolver */

/*@} */

/**@name Type Definitions */
/*@{ */


/*@} */

} /* namespace Kratos.*/

#endif /* KRATOS_ROM_BUILDER_AND_SOLVER  defined */


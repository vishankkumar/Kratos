from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics
import KratosMultiphysics.FemToDemApplication as KratosFemDem
import KratosMultiphysics.FemToDemApplication.MainCouplingFemDem as MainCouplingFemDem

def Wait():
    input("Press Something")

#============================================================================================================================
class MainCoupledFemDemSubstepping_Solution(MainCouplingFemDem.MainCoupledFemDem_Solution):
#============================================================================================================================

#============================================================================================================================
    def SolveSolutionStep(self):  # Method to perform the coupling FEM <-> DEM

        self.FEM_Solution.clock_time = self.FEM_Solution.StartTimeMeasuring()

        #### SOLVE FEM #########################################
        self.FEM_Solution.solver.Solve()
        ########################################################

        self.ExecuteBeforeGeneratingDEM()
        self.GenerateDEM() # we create the new DEM of this time step
        self.ExecuteAfterGeneratingDEM()

        # Perform substepping
        pseudo_substepping_time = 0
        if self.DEM_Solution.spheres_model_part.NumberOfElements() > 0:
            while pseudo_substepping_time <= self.FEM_Solution.delta_time:
                self.BeforeSolveDEMOperations()

                #### SOLVE DEM #########################################
                self.DEM_Solution.solver.Solve()
                ########################################################

                self.DEM_Solution.FinalizeSolutionStep()
                self.DEM_Solution.solver._MoveAllMeshes(self.DEM_Solution.time, self.DEM_Solution.solver.dt)

                # We reset the position of the slave DEM
                self.UpdateDEMVariables()

                # DEM GiD print output
                self.PrintDEMResults()

                self.DEM_Solution.FinalizeTimeStep(self.DEM_Solution.time)

                # Advancing in DEM explicit scheme
                pseudo_substepping_time += self.DEM_Solution.solver.dt
        
        else: # In case there are no DEM yet
            self.OnlyUpdateTimeAndStepInDEM()



        # Transfer the contact forces of the DEM to the FEM nodes
        if self.TransferDEMContactForcesToFEM:
            self.TransferNodalForcesToFEM()

        self.FEM_Solution.StopTimeMeasuring(self.FEM_Solution.clock_time,"Solving", False)

        # Update Coupled Postprocess file for Gid (post.lst)
        self.WritePostListFile()

        # Print required info
        self.PrintPlotsFiles()

#============================================================================================================================
    def BeforeSolveDEMOperations(self):
        self.DEM_Solution.time += self.DEM_Solution.solver.dt
        self.DEM_Solution.step += 1
        self.DEM_Solution.DEMFEMProcedures.UpdateTimeInModelParts(self.DEM_Solution.all_model_parts,
                                                                   self.DEM_Solution.time,
                                                                   self.DEM_Solution.solver.dt,
                                                                   self.DEM_Solution.step,
                                                                   self.DEM_Solution.IsTimeToPrintPostProcess())

#============================================================================================================================
    def OnlyUpdateTimeAndStepInDEM(self):
        self.DEM_Solution.time = self.FEM_Solution.time
        self.DEM_Solution.step = self.FEM_Solution.step
        self.DEM_Solution.DEMFEMProcedures.UpdateTimeInModelParts(self.DEM_Solution.all_model_parts,
                                                                   self.DEM_Solution.time,
                                                                   self.DEM_Solution.solver.dt,
                                                                   self.DEM_Solution.step,
                                                                   self.DEM_Solution.IsTimeToPrintPostProcess())

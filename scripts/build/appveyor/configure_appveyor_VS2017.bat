cmake .. -G "Visual Studio 15 2017 Win64"                                       ^
-DCMAKE_CXX_FLAGS="-msse3" 								                        ^
-DCMAKE_C_FLAGS="-msse3   " 									                ^
-DBOOST_ROOT="C:\Libraries\boost_1_65_1"                                        ^
-DBOOST_PYTHON_SUFFIX="3-vc141-mt-gd-1_65_1"                                    ^
-DPYTHON_INCLUDE_PATH="C:\Python36-x64\include"							        ^
-DPYTHON_LIBRARY="C:\Python36-x64\lib\python36.lib"					            ^
-DCMAKE_BUILD_TYPE="Debug"  													^
-DDEM_APPLICATION=ON                                                            ^
-DEXTERNAL_SOLVERS_APPLICATION=OFF                                               ^
-DFLUID_DYNAMICS_APPLICATION=ON                                                 ^
-DSTRUCTURAL_MECHANICS_APPLICATION=ON                                           ^
-DSWIMMING_DEM_APPLICATION=OFF                                                  ^
-DMKLSOLVER_INCLUDE_DIR="UNSET"                                                 ^
-DMKLSOLVER_LIB_DIR="UNSET"                                                     ^
-DMETIS_APPLICATION=OFF                                                         ^
-DPARMETIS_ROOT_DIR="UNSET"                                                     ^
-DTRILINOS_APPLICATION=OFF                                                      ^
-DTRILINOS_ROOT="UNSET"                                                         ^
-DINSTALL_EMBEDDED_PYTHON=ON                                                    ^
-DINCLUDE_FEAST=OFF                                                              
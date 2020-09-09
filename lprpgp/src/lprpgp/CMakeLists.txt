PROJECT(lprpg)

set( CMAKE_MODULE_PATH    ${CMAKE_MODULE_PATH}
                          ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules
                          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

IF(NOT DISABLE_CLP)
find_package(Clp)
find_package(CoinUtils)
find_package(Cbc)
find_package(Cgl)
find_package(OSI)
ENDIF(NOT DISABLE_CLP)

find_package(LPSolve)
find_package(CBLAS)
find_package(CLapack)

set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

include_directories(${val_SOURCE_DIR} ${val_SOURCE_DIR}/parsing ${val_BINARY_DIR} ${CLP_INCLUDES}/coin ${CBC_INCLUDES}/coin ${CPLEX_INCLUDES} ${CONCERT_INCLUDES} ${LPSOLVE_INCLUDES})

set(lprpg_build_srcs
    PreferenceHandler.cpp
    Decomposition.cpp 
    GlobalSchedule.cpp 
    RPGBuilder.cpp
    RPGBuilderAnalysis.cpp 
    FFSolver.cpp 
    NumericAnalysis.cpp
    landmarksanalysis.cpp
    MILPRPG.cpp
    MetricRPG.cpp
    NNF.cpp
    LiteralAnalysis.cpp
    solver.cpp
)

add_library(LPRPGCommon STATIC ${lprpg_build_srcs})

target_link_libraries(LPRPGCommon ParsePDDL Inst)

if (CLP_INCLUDES AND CBC_INCLUDES)    
    ADD_EXECUTABLE(lprpgp-clp lprpgMain.cpp solver-clp.cpp solver-clp-globals.cpp)
    
    target_link_libraries(lprpgp-clp
        LPRPGCommon
        ${CBC_SOLVER_LIBRARY} ${CBC_LIBRARY} ${CGL_LIBRARIES} ${OSICLP_LIBRARIES} ${OSI_LIBRARIES} ${CLP_LIBRARIES}  ${COINUTILS_LIBRARIES}
    )
                
endif (CLP_INCLUDES AND CBC_INCLUDES)

if (LPSOLVE_INCLUDES AND CBLAS_LIBRARIES AND CLAPACK_LIBRARIES)
    ADD_EXECUTABLE(lprpgp-lpsolve lprpgMain.cpp solver-lpsolve.cpp solver-lpsolve-globals.cpp)
    
    target_link_libraries(lprpgp-lpsolve
        LPRPGCommon
        ${LPSOLVE_LIBRARIES} ${CBLAS_LIBRARIES} colamd dl
    )
                
endif (LPSOLVE_INCLUDES AND CBLAS_LIBRARIES AND CLAPACK_LIBRARIES)

if(CPLEX AND ILOCPLEX AND CONCERT AND CPLEX_INCLUDES AND CONCERT_INCLUDES)
    ADD_EXECUTABLE(lprpgp-cplex lprpgMain.cpp solver-cpx.cpp solver-cpx-globals.cpp solver-cpx-paramils.cpp)
    
    target_link_libraries(lprpgp-cplex    
        LPRPGCommon
        ${ILOCPLEX} ${CPLEX} ${CONCERT} pthread
    )
    
endif(CPLEX AND ILOCPLEX AND CONCERT AND CPLEX_INCLUDES AND CONCERT_INCLUDES)
    

if(CPLEX AND ILOCPLEX AND CONCERT AND CPLEX_INCLUDES AND CONCERT_INCLUDES AND CLP_INCLUDES AND CBC_INCLUDES)
    ADD_EXECUTABLE(lprpgp-both lprpgMain.cpp solver-cpx.cpp solver-cpx-paramils.cpp solver-clp.cpp solver-both.cpp)
    
    target_link_libraries(lprpgp-both
        LPRPGCommon
        ${CBC_SOLVER_LIBRARY} ${CBC_LIBRARY} ${CGL_LIBRARIES} ${OSICLP_LIBRARIES} ${OSI_LIBRARIES} ${CLP_LIBRARIES}  ${COINUTILS_LIBRARIES}
        ${ILOCPLEX} ${CPLEX} ${CONCERT} pthread
    )
endif(CPLEX AND ILOCPLEX AND CONCERT AND CPLEX_INCLUDES AND CONCERT_INCLUDES AND CLP_INCLUDES AND CBC_INCLUDES)


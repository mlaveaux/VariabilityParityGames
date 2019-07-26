echo "Verify Zlnk Bdd"
./verify.sh VPGSolver_bdd | grep -v Start
echo "Verify Zlnk Bdd prio compress"
./verify.sh VPGSolver_bdd p | grep -v Start
echo "Verify FPite"
./verify.sh VPGSolver_bdd F | grep -v Start
echo "Verify Zlnk Bdd single"
./verify_single.sh VPGSolver_bdd_single | grep -v Start
echo "Verify FPite Single"
./verify_single.sh VPGSolver_bdd_single F | grep -v Start
echo "Verify FPIte Single local"
./verify_single_local.sh VPGSolver_bdd_single "F" | grep -v Start
echo "Verify Single local"
./verify_local.sh VPGSolver_bdd "F" | grep -v Start
echo "Verify FPIte assisted"
./verify_single_assisted.sh VPGSolver_bdd_single F | grep -v Start
echo "Verify FPIte local"
./verify_local.sh VPGSolver_bdd "F" | grep -v Start
echo "Verify Zlnk Bdd Explicit"
./verify.sh VPGSolver_explicit | grep -v Start
echo "Verify PG parsing"
./verify_PG.sh VPGSolver_bdd_single "F"|grep -v Start

#! /bin/sh

# Get the OOMPH-LIB root directory from a makefile
OOMPH_ROOT_DIR=$1


#Set the number of tests to be checked
NUM_TESTS=1


# Setup validation directory
#---------------------------
touch Validation
rm -r -f Validation
mkdir Validation

# Validation for demo adaptive unsteady heat
#-------------------------------------------
cd Validation

echo "Running adaptive 2D tanh unsteady heat validation "
mkdir RESLT
../two_d_unsteady_heat_adapt > OUTPUT_orig
echo "done initial run"
mv RESLT RESLT_orig
mkdir RESLT
../two_d_unsteady_heat_adapt  RESLT_orig/restart3.dat > OUTPUT_restarted
echo "done restarted run"
echo " " >> validation.log
echo "Adaptive 2D tanh unsteady heat validation " >> validation.log
echo "-----------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log


cat RESLT_orig/soln1.dat RESLT_orig/soln3.dat RESLT/soln3.dat RESLT/soln6.dat \
    > result.dat

if test "$2" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
$OOMPH_ROOT_DIR/scripts/fpdiff.py ../validata/result.dat.gz \
    result.dat  >> validation.log
fi

# Append output to global validation log file
#--------------------------------------------
cat validation.log >> $OOMPH_ROOT_DIR/validation.log

cd ..



#######################################################################


#Check that we get the correct number of OKs
# validate_ok_count will exit with status
# 0 if all tests has passed.
# 1 if some tests failed.
# 2 if there are more 'OK' than expected.
. $OOMPH_ROOT_DIR/scripts/validate_ok_count

# Never get here
exit 10

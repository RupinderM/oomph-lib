#! /bin/sh

# Get the OOPMH-LIB root directory from a makefile
OOMPH_ROOT_DIR=$1


#Set the number of tests to be checked
NUM_TESTS=1


# Setup validation directory
#---------------------------
touch Validation
rm -r -f Validation
mkdir Validation

# Validation for demo poisson
#----------------------------
cd Validation

echo "Running 2D demo poisson with flux bc validation "
mkdir RESLT
../two_d_poisson_flux_bc > OUTPUT_demo_poisson
echo "done"
echo " " >> validation.log
echo "2D demo 2D poisson with flux bc validation" >> validation.log
echo "------------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat RESLT/soln1.dat RESLT/soln2.dat \
    RESLT/soln3.dat  > two_d_poisson_results.dat

if test "$2" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
$OOMPH_ROOT_DIR/scripts/fpdiff.py \
../validata/two_d_poisson_results.dat.gz \
    two_d_poisson_results.dat  >> validation.log
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

#!/usr/bin/env python3


# TODO:

# Different message for dummy oks due to missing code? But we can't see
# what's missing from outside the code. So unfortunately can't do anything
# fancy with hypre tests etc..

# Add a check for arpack... at the moment we just assume it's not there
# because I don't know how to look for it!


# ASSUMPTIONS:

# All validate.sh scripts return an appropriate exit status. Otherwise this
# script cannot know if the test failed!

# Driver requirements are currently determined by:
# * mpi driver <=> has "mpi" in the path.
# * arpack drivers <=> has "eigenproblem" in the path.
# but this is easy to change by modifying e.g. check_if_mpi_driver(...)


# Some python 3 compatability. With these imports most scripts should work
# in both python 2.7 and python 3.x.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import
from __future__ import unicode_literals

# Other compatability notes:

# Anything passed to/from subprocesses as a stream must be encoded (with
# x.encode()) / decoded (with str(y)) in python3. e.g. see
# variable_from_makefile.


import subprocess as subp
from multiprocessing import Pool
import sys
import argparse
import os
import os.path
import multiprocessing
import itertools as it
import pprint

from functools import partial as pt
from os.path import join as pjoin


class Colours:
    """Very simple class for coloring in text output. Stolen from
    http://stackoverflow.com/questions/287871/print-in-terminal-with-colors-using-python

    If we were using python 3 we could use the termcolor package instead."""
    Header = '\033[96m'
    Okgreen = '\033[92m'
    MakeFail = '\033[93m'
    TestFail = '\033[91m'
    Endc = '\033[0m'

    def disable(self):
        self.Header = ''
        self.Okgreen = ''
        self.MakeFail = ''
        self.TestFail = ''
        self.Endc = ''


# Make a GLOBAL colours object to tell print functions how to make things
# pretty. Easier to make it global because we want to be able to disable it
# from inside main() without passing around a Colours object all over the
# place.
COLOURS = Colours()

# Various functions for printing with pretty colours


def highlight(directorypath):
    return os.path.dirname(directorypath) + "/" + COLOURS.Header + \
        os.path.basename(directorypath) + COLOURS.Endc


def build_fail_message(directory):
    print(COLOURS.MakeFail+"[BUILD FAIL] " + highlight(directory) +
          COLOURS.Endc)


def check_fail_message(directory):
    print(COLOURS.TestFail+"[FAILED]     " + highlight(directory) +
          COLOURS.Endc)


def check_success_message(directory):
    print(COLOURS.Okgreen + "[OK]         " + highlight(directory) +
          COLOURS.Endc)


def no_check_message(directory):
    print(COLOURS.Okgreen + "[NO CHECK]   " + highlight(directory) +
          COLOURS.Endc)


def missing_feature_message(directory, feature):
    print(COLOURS.Okgreen + "[NO " + feature.upper() + "]     " +
          highlight(directory) + COLOURS.Endc)


# =============================================================================
# Some general utility functions
# =============================================================================

class NoCMakeScriptError(IOError):
    pass


def variable_from_cmake_script(variable_name: str,
                               cmakelist_path: str = "CMakeLists.txt"):
    """Extract a variable from a makefile (using make).

    The basic idea is to cat a new "print-var" command, which prints the
    variable we want, onto the start of the makefile. Then we just run
    "make print-var" and make gives us the value we want.

    Of course we don't actually modify the real makefile, we make a
    stream and pipe it into make instead.

    The idea came from somewhere on the internet, probably stackoverflow.
    """

    if not os.path.isfile(cmakelist_path):
        raise NoCMakeScriptError(
            f"CMakeLists.txt file not found at path: {cmakelist_path} , \
                maybe you haven't built the Makefile yet or you are in the \
                    wrong folder?")

    # Find out which directory it's in (and handle the case where it's in
    # the pwd, which dirname fails a bit on...
    cmakefile_dir = os.path.dirname(cmakelist_path)
    if cmakefile_dir == "":
        cmakefile_dir = pjoin('.', '')

    # Run make using both a real makefile and a dummy one we are about to
    # create. Call the "print-var" command which will be in the dummy
    # makefile.
    process = subp.Popen(["make", "-f", "-", "-f", cmakelist_path, "print-var"],
                         cwd=cmakefile_dir,
                         stdin=subp.PIPE, stdout=subp.PIPE)

    # Send in a dummy makefile with a print command, output should be the
    # value of the variable.
    stdout, _ = process.communicate(
        ("print-var:; @echo $(" + variable_name + ")").encode())

    # Check that make exited with a success code (0)
    returncode = process.wait()
    if returncode != 0:
        raise subp.CalledProcessError(returncode, "make print-var ....")

    # Convert to python string (for python3), strip trailing
    # newline/whitespace and return.
    return stdout.decode().rstrip()


class NoMakefileError(IOError):
    pass


def variable_from_makefile(variable_name, makefile_path="Makefile"):
    """Extract a variable from a makefile (using make).

    The basic idea is to create a new "print-var" command, which prints the
    variable we want, onto the start of the makefile. Then we just run
    "make print-var" and make gives us the value we want.

    Of course we don't actually modify the real makefile, we make a
    stream and pipe it into make instead.

    The idea came from somewhere on the internet, probably
    stackoverflow."""

    if not os.path.isfile(makefile_path):
        raise NoMakefileError("Makefile not found at path: " + makefile_path +
                              " , maybe you haven't built the Makefile yet or" +
                              " you are in the wrong folder?")

    # Find out which directory it's in (and handle the case where it's in
    # the pwd, which dirname fails a bit on...
    makefile_dir = os.path.dirname(makefile_path)
    if makefile_dir == "":
        makefile_dir = pjoin('.', '')

    # Run make using both a real makefile and a dummy one we are about to
    # create. Call the "print-var" command which will be in the dummy
    # makefile.
    process = subp.Popen(["make", "-f", "-", "-f", makefile_path, "print-var"],
                         cwd=makefile_dir,
                         stdin=subp.PIPE, stdout=subp.PIPE)

    # Send in a dummy makefile with a print command, output should be the
    # value of the variable.
    stdout, _ = process.communicate(
        ("print-var:; @echo $(" + variable_name + ")").encode())

    # Check that make exited with a success code (0)
    returncode = process.wait()
    if returncode != 0:
        raise subp.CalledProcessError(returncode, "make print-var ....")

    # Convert to python string (for python3), strip trailing
    # newline/whitespace and return.
    return stdout.decode().rstrip()


def error(*args):
    """Write an error message to stderr and exit."""
    sys.stderr.write("\nERROR:\n" + "\n".join(args) + "\n")
    sys.exit(2)


def get_oomph_root():
    try:
        print("Trying to extract the root dir from a Makefile in the pwd.")
        oomph_root = variable_from_makefile("abs_top_srcdir")
        print("Extracted oomph_root = " + oomph_root)
        return oomph_root
    # If no makefile exists we are stuck:
    except NoMakefileError as e:
        print(str(e))
        error("You must either run this script from within an oomph-lib",
              "directory or specify the path to oomph_root using -C.")


# =============================================================================
# Validation functions
# =============================================================================


def find_validate_dirs(base_dirs):
    """Construct a list of validation directories by searching for
    validate.sh scripts."""

    all_validation_dirs = []
    for base in base_dirs:
        for root, dirs, files in os.walk(base):
            if 'validate.sh' in files:
                all_validation_dirs.append(root)
    return all_validation_dirs


def dispatch_dir(dirname, features, **kwargs):
    """Check for missing features and print the appropriate message if
    needed. Otherwise run the check function."""

    # For each possible feature in features check if we have it. If not
    # check if this directory needs it and if so print a message, otherwise
    # run the check.
    for feature in features:
        if not feature['have_feature']:
            if feature['check_driver_function'](dirname):
                missing_feature_message(dirname, feature['feature_name'])
                return
    make_check_in_dir(dirname, **kwargs)


# Functions for checking if a test needs a certain feature
def check_if_mpi_driver(directories: list):
    return "mpi" in directories


def check_if_arpack_driver(directories: list):
    return "eigenproblems" in directories


def check_if_hlib_driver(directories: list):
    # hlib is in "oomphlib", so take it out in case people use dirs called
    # oomphlib with no -.
    return "hlib" in directories.replace("oomphlib", "")


# The function doing the bulk of the actual work (called many times in
# parallel by main).
def make_check_in_dir(directory, just_build=False):
    """
    Rebuild binaries in the directory using make if needed then run the
    tests.

    Since everything important is written to validation logs we just
    summarise passes/fails on stdout.

    Output from compilation is sent to make_check_output file in
    directory. The file is then moved into Validation once we can be
    sure the folder exists.
    """

    # Write make output into a file in test rootdir (in case Validation dir
    # doesn't exist yet).
    tracefile_path = pjoin(directory, "make_check_output")

    with open(tracefile_path, 'w') as tracefile:

        # Need to flush write buffer so that this gets into the file
        # before the build output.
        tracefile.write("Building WITH FAKE TEST PASS CONDITION:\n")
        tracefile.flush()

        # Rebuild (but don't run the test yet). Minimal output except for
        # errors. Output goes to the trace file.
        build_return = subp.call(
            'make check --silent LIBTOOLFLAGS=--silent TESTS_ENVIRONMENT=true',
            shell=True, cwd=directory, stdout=tracefile, stderr=subp.STDOUT)

        # If it failed then return a build failure immediately
        if build_return != 0:
            build_fail_message(directory)
            return

        if not just_build:
            tracefile.write("\nRunning self test properly:\n")
            tracefile.flush()

            # Run make check (runs the actual test)
            test_result = subp.call('make check --silent LIBTOOLFLAGS=--silent',
                                    shell=True,
                                    cwd=directory,
                                    stdout=tracefile,
                                    stderr=subp.STDOUT)

        else:
            tracefile.write(
                "\nNot running self test because you set the 'just_build' option\n")
            tracefile.flush()

            test_result = 0

    final_tracefile_path = pjoin(directory, "Validation", "make_check_output")

    # Validation dir should exist now. Move the output file there if so,
    # otherwise issue a warning.
    if not os.path.isdir(os.path.dirname(final_tracefile_path)):
        sys.stderr.write("Warning: no Validation directory in "+directory
                         + " so I couldn't put make_check_output in there\n")
        sys.stderr.flush()
    else:
        os.rename(tracefile_path, final_tracefile_path)

    if test_result == 0:
        check_success_message(directory)
        return
    else:
        check_fail_message(directory)
        return


# Function dealing with parsing of arguments, getting everything set up and
# dispatching jobs to processes.
def main():
    """
    Run oomph-lib self tests in parallel.

    Typical usage is just:

        parallel_self_test.py

    from within an oomph-lib directory, or

        parallel_self_test.py -C /abs/path/to/oomph/root

    from anywhere.

    For this script to work correctly ALL validate.sh scripts must return
    an exit status. Use the --check-scripts option to check this.

    Note: aborting with C-c will not work in python2 due to limitations
    of python2's multiprocessing module. The easiest way to abort is
    probably to kill the terminal emulator itself. Alternatively
    background the python process with C-z then kill it (i.e. run "kill
    %%"). Or just upgrade to python3.
    """

    # Parse inputs
    # ============================================================
    parser = argparse.ArgumentParser(
        description=main.__doc__,

        # Don't mess up my formating in the help message
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    # make uses -C for "run make in this directory" so copy it
    parser.add_argument('-C', dest='oomph_root',
                        help='Set the root directory of oomph-lib, by default try to \
    extract it from a Makefile.')

    parser.add_argument('-a', action='store_true', dest='makeclean',
                        help='Run make clean on test folders before starting.')

    parser.add_argument('-l', '--base-dir', action='append', dest='base_dirs',
                        default=[],
                        help='Specify directories relative to the root to (recursively)'
                        + ' look for tests in.'
                        + ' Uses "demo_drivers" and "self_test" by default.')

    parser.add_argument('-L', '--base-dirs-file', action='append', dest='base_dirs_files',
                        default=[],
                        help='Specify files containing a list of directories relative to the root to (recursively) look for tests in. Uses "demo_drivers" and "self_test" by default.')

    parser.add_argument('-j', '-n', dest='ncores',
                        help='Specify how many cores should be used altogether \
                        (taking mpi runs into account). By default use all cores.',
                        default=multiprocessing.cpu_count())

    parser.add_argument('--no-colour', action='store_true',
                        help='Disable colours in output.')

    parser.add_argument('--check-scripts', action='store_true',
                        help='Check all the validate.sh scripts using a simple '
                        + 'regex to make sure that they set an exit status.')

    parser.add_argument('--just-build', action='store_true',
                        help="Only build the tests, don't run them")

    parser.add_argument('--serial-mode', action='store_true',
                        help='Run the script without parallelism (for debugging'
                        + ' purposes).')

    args = parser.parse_args()

    if args.no_colour:
        COLOURS.disable()

    # Attempt to get the oomph-lib root dir from a Makefile in the current
    # directory.
    if args.oomph_root is None:
        args.oomph_root = get_oomph_root()

    # If we requested just checking the validate.sh scripts instead of
    # running the tests
    if args.check_scripts:
        # Grep for some way of returning an exit status in validate.sh scripts,
        # print out those that don't contain one.
        print("Checking validate.sh scripts. Any scripts printed below do not set\n" +
              "their exit status properly and so the results cannot be correctly\n" +
              "reported by this script.")
        print("Look in other validate scripts to see how to fix this.")
        subp.call('find -name "validate.sh" | xargs grep -i -L "^exit \|^set -o errexit"',
                  shell=True, cwd=args.oomph_root)
        return 0

    # =========================================================================
    # Figure out if we have various features
    # =========================================================================

    # ??ds there MUST be a way to detect this somehow...
    have_arpack = False

    # Find out if we have mpi by looking for "OOMPH_HAS_MPI" in flags in
    # Makefile.
    have_mpi = "OOMPH_HAS_MPI" in variable_from_makefile(
        "AM_CPPFLAGS", pjoin(args.oomph_root, "Makefile"))

    # Similarly for hlib
    have_hlib = "OOMPH_HAS_HLIB" in variable_from_makefile(
        "AM_CPPFLAGS", pjoin(args.oomph_root, "Makefile"))

    # List of possible features. Each one must contain: "feature_name",
    # check_driver_function--a function to find out if a directory requires
    # this feature and have_feature--a boolean for if we have this feature
    # or not.
    oomph_features = ([
        {
            'feature_name': "arpack",
            'check_driver_function': check_if_arpack_driver,
            'have_feature': have_arpack
        },
        {
            'feature_name': "mpi",
            'check_driver_function': check_if_mpi_driver,
            'have_feature': have_mpi
        },
        {
            'feature_name': "hlib",
            'check_driver_function': check_if_hlib_driver,
            'have_feature': have_hlib
        }
    ])

    # Print our findings:
    print("\nChecked for the following features:")
    for feature in oomph_features:
        print("    ", feature['feature_name'], ":", feature['have_feature'])

    # =========================================================================
    # Gather directory lists etc.
    # =========================================================================
    base_dirs = args.base_dirs

    # If given any files with lists of dirnames then read in those too
    for dirs_file_name in args.base_dirs_files:
        with open(dirs_file_name) as f:
            # Strip whitespace and ignore blank lines
            base_dirs = base_dirs + \
                [l.strip() for l in f.readlines() if l.strip() != ""]

    # If there are no base dirs given in either list then use defaults:
    if len(base_dirs) == 0:
        base_dirs = ["demo_drivers", "self_test"]

    # Convert to absolute paths
    abs_base_dirs = [os.path.abspath(os.path.join(args.oomph_root, b))
                     for b in base_dirs]

    print("\nLooking for validate.sh scripts in directories:")
    pprint.pprint(abs_base_dirs)
    print()

    # =========================================================================
    # Run tests
    # =========================================================================

    # Clean up from past runs if requested
    if args.makeclean:
        print("Running (recursive) make clean in", *abs_base_dirs)
        for directory in abs_base_dirs:
            # Make clean in "directory" with stdout thrown away.
            subp.check_call(['make', 'clean', '-k',
                             '-j', str(args.ncores)],
                            stdout=open(os.devnull, 'w'),
                            cwd=directory)

    # Construct a list of validation directories
    validation_dirs = find_validate_dirs(abs_base_dirs)

    # Construct final function to run on each directory
    f = pt(dispatch_dir, features=oomph_features, just_build=args.just_build)

    if args.serial_mode:
        list(map(f, validation_dirs))  # list forces evaluation of the map
    else:
        # Run it in parallel.
        Pool(processes=int(args.ncores)).map(f, validation_dirs, 1)
        # Set chunksize to 1 (i.e. each "make check" call is in its own "chunk
        # of work") to avoid the situation where multiple slow "make check"s
        # end up in the same chunk and we have to wait ages for it to finish.

    # Done!
    return 0


# If this script is run from a shell then run main() and return the result.
if __name__ == "__main__":
    sys.exit(main())
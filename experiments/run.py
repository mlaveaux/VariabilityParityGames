#!/usr/bin/env python

import os
import subprocess

# export PATH=/d/Repositories/mCRL2-build/stage/bin/RelWithDebInfo/:$PATH
# export PATH=/d/Repositories/VariabilityParityGames/implementation/VPGSolver-build/RelWithDebInfo/:$PATH

case_path = 'D:\\Repositories\\VariabilityParityGames\\cases\\testsuite\\elevator\\game_prop1\\SVPG'


def main():
    subprocess.call(['VPGSolver_explicit', case_path])

if __name__ == "__main__":
    main()

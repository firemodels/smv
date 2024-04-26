from sys import platform
import sys
import subprocess
import shlex
from functools import partial
from tkinter import *
import os

root = Tk()

# directory locations

script_dir = os.path.dirname(os.path.realpath(__file__))


button_width=13

# link windows batch files to python commands

def edit_case(): subprocess.Popen(["notepad","simple1.fds",""])
def run_case():  subprocess.Popen(["fds_local.bat","simple1.fds",""])
cmd = "smokeview simple1"
args = shlex.split(cmd)
def view_case(): subprocess.Popen(args)
#def view_case(): os.system("smokeview simple1")

#cmd = "smokeview simple1"
#args = shlex.split(cmd)
#p = sub.Popen(args,,)
 
root.title('FDS Demo')
root.resizable(0, 0)

R=0
R=R+1
Button(root, text="Edit case",  width=button_width, command=edit_case).grid(row=R, column=1)
Button(root, text="Run case",   width=button_width, command=run_case).grid(row=R,  column=2)
Button(root, text="View case",  width=button_width, command=view_case).grid(row=R, column=3)

root.mainloop()

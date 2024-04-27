from sys import platform
import sys
import subprocess
import shlex
from functools import partial
import os

if (sys.platform == "win32"): 
  from tkinter import *
else:
  from Tkinter import *

root = Tk()

# directory locations

script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(script_dir)

#update path for fds (batch files for now were not working when called from python)
if (sys.platform == "win32"): 
  os.environ["I_MPI_ROOT"]="C:\\Program Files\\firemodels\\FDS6\\bin\\mpi"
  os.environ["PATH"]=os.environ["I_MPI_ROOT"]+";"+os.environ["PATH"]
  os.environ["MPIEXEC_PORT_RANGE"]=""
  os.environ["MPICH_PORT_RANGE"]=""
  MAKECASE="makecase"
  fdsrun="fdsrun.bat"
else:
  MAKECASE="./makecase.sh"
  fdsrun="./fdsrun.sh"

button_width=8
edit_width=10

# build the case (in foreground - but finishes instantly)
def make_case1(): os.system(MAKECASE + " " + fire_size.get() + " " + door_height.get() + " " + gravx.get() + " " + gravy.get() + " " + gravz.get() + " case1")
def make_case2(): os.system(MAKECASE + " " + fire_size.get() + " " + door_height.get() + " " + gravx.get() + " " + gravy.get() + " " + gravz.get() + " case2")

# run the case (in background)
cmdfds1 = fdsrun + " case1.fds"
argsfds1 = shlex.split(cmdfds1)
def run_case1(): subprocess.Popen(argsfds1)

cmdfds2 = fdsrun + "case2.fds"
argsfds2 = shlex.split(cmdfds2)
def run_case2(): subprocess.Popen(argsfds2)

# view the case (in background)
cmdsmv1 = "smokeview case1"
argssmv1 = shlex.split(cmdsmv1)
def view_case1(): subprocess.Popen(argssmv1)

cmdsmv2 = "smokeview case2"
argssmv2 = shlex.split(cmdsmv2)
def view_case2(): subprocess.Popen(argssmv2)

root.title('Fire Smoke Demo')
root.resizable(0, 0)

R=0

Label(root, text="fire size (kW):").grid(sticky="e",row=R, column=0)
fire_size=Entry(root, width = edit_width)
fire_size.grid(sticky="e",row=R, column=1)
fire_size.insert(0,"1000")

R=R+1
Label(root, text="door height (m):").grid(sticky="e",row=R, column=0)
door_height=Entry(root, width = edit_width)
door_height.grid(sticky="e",row=R, column=1)
door_height.insert(0,"2.0")

R=R+1
Label(root, text="gravity (m/s2) x,y,z:").grid(row=R, column=0)
gravx=Entry(root, width = edit_width)
gravx.grid(sticky="e",row=R, column=1)
gravx.insert(0,"0.0")
gravy=Entry(root, width = edit_width)
gravy.grid(sticky="e",row=R, column=2)
gravy.insert(0,"0.0")
gravz=Entry(root, width = edit_width)
gravz.grid(sticky="e",row=R, column=3)
gravz.insert(0,"-9.8")

#R=R+1
#option1 = IntVar()
#Chkoption1 = Checkbutton(root, text="option1", width = 15, variable = option1).grid(row=R, column=0)

R=R+1
Label(root, text="case 1:").grid(sticky="e",row=R, column=0)
Button(root, text="Make",  width=button_width, command=make_case1).grid(row=R, column=1)
Button(root, text="Run",   width=button_width, command=run_case1).grid(row=R,  column=2)
Button(root, text="View",  width=button_width, command=view_case1).grid(row=R, column=3)

R=R+1
Label(root, text="case 2:").grid(sticky="e",row=R, column=0)
Button(root, text="Make",  width=button_width, command=make_case2).grid(row=R, column=1)
Button(root, text="Run",   width=button_width, command=run_case2).grid(row=R,  column=2)
Button(root, text="View",  width=button_width, command=view_case2).grid(row=R, column=3)

root.mainloop()

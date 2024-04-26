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
os.chdir(script_dir)

#update path for fds (batch files for now were not working when called from python)
if (sys.platform == "win32"): 
  os.environ["I_MPI_ROOT"]="C:\\Program Files\\firemodels\\FDS6\\bin\\mpi"
  os.environ["PATH"]=os.environ["I_MPI_ROOT"]+";"+os.environ["PATH"]
  os.environ["MPIEXEC_PORT_RANGE"]=""
  os.environ["MPICH_PORT_RANGE"]=""
else:
  MAKECASE="makecase.sh"

button_width=8
edit_width=10

# build the case (in foreground - but finishes instantly)
def make_case(): os.system("makecase " + fire_size.get() + " " + door_height.get() + " " + gravx.get() + " " + gravz.get())

# run the case (in background)
cmdfds = "fds simple1.fds"
argsfds = shlex.split(cmdfds)
def run_case(): subprocess.Popen(argsfds)

# view the case (in background)
cmdsmv = "smokeview simple1"
argssmv = shlex.split(cmdsmv)
def view_case(): subprocess.Popen(argssmv)

root.title('FDS Demo')
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
Label(root, text="gravity x (m/s2):").grid(row=R, column=0)
gravx=Entry(root, width = edit_width)
gravx.grid(sticky="e",row=R, column=1)
gravx.insert(0,"0.0")

R=R+1
Label(root, text="gravity z (m/s2):").grid(sticky="e",row=R, column=0)
gravz=Entry(root, width = edit_width)
gravz.grid(sticky="e",row=R, column=1)
gravz.insert(0,"-9.8")

#R=R+1
#option1 = IntVar()
#Chkoption1 = Checkbutton(root, text="option1", width = 15, variable = option1).grid(row=R, column=0)

R=R+1
Button(root, text="Make case",  width=button_width, command=make_case).grid(row=R, column=0)
Button(root, text="Run case",   width=button_width, command=run_case).grid(row=R,  column=1)
Button(root, text="View case",  width=button_width, command=view_case).grid(row=R, column=2)

root.mainloop()

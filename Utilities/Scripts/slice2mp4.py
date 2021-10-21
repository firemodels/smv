from tkinter import *

master = Tk()

R=0
colorbar= IntVar()
Checkbutton(master, text="Show Colorbar", variable=colorbar).grid(row=R, sticky=W)

R=R+1
timebar= IntVar()
Checkbutton(master, text="Show Timebar", variable=timebar).grid(row=R, sticky=W)


def helloCallBack():
   showinfo( "Hello Python", "Hello World")


R=R+1
Label(master, text="font size:").grid(column=0, row=R)
fontsize=IntVar()
fontsize.set(2)

Radiobutton(master, 
               text="Small",
               padx = 0, 
               variable=fontsize, 
               value=1).grid(row=R, column=1)
Radiobutton(master, 
               text="Large",
               padx = 0, 
               variable=fontsize, 
               value=2).grid(row=R, column=2)

R=R+1
Button(master, text="Save settings",    width=20, command=helloCallBack).grid(row=R, column=0, columnspan=3)

R=R+1
Button(master, text="Create MP4 Animation",    width=20, command=helloCallBack).grid(row=R, column=0, columnspan=3)

mainloop()

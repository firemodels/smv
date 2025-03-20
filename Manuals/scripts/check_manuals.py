# -*- coding: utf-8 -*-
"""
Created on Wed Mar 12 11:20:18 2025

@author: jhodges
"""

import os, subprocess, platform, argparse, glob, sys

def sortWhitelist(firemodels):
    whitelist = os.path.join(firemodels, 'fds','Manuals','Bibliography')+ os.sep + 'whitelist.txt'
    whitelist = os.path.abspath(whitelist)
    
    with open(whitelist,'r') as f:
        txt = f.readlines()
    
    words = list(set(txt[1:]))
    words.sort()
    
    outtxt = txt[0] + '\n' + '\n'.join(words) + '\n'
    while '\n\n' in outtxt:
        outtxt = outtxt.replace('\n\n','\n')
        
    with open(whitelist.replace('.txt','_back.txt'), 'w') as f:
        f.writelines(txt)
        
    with open(whitelist, 'w') as f:
        f.write(outtxt)

def checkSpelling(file, firemodels):
    whitelist = os.path.join(firemodels, 'fds','Manuals','Bibliography')+ os.sep + 'whitelist.txt'
    whitelist = os.path.abspath(whitelist)
    if 'Windows' in platform.platform():
        whitelist = whitelist.replace('\\','/')
        whitelist = whitelist[0].upper() + whitelist[1:]
    cmd = ['aspell','--lang=en','--mode=tex','--add-extra-dicts=%s'%(whitelist), 'list', '<', file]
    
    p = subprocess.run(cmd, capture_output=True, shell=True)
    txt = p.stdout.decode('utf-8')
    txt = txt.replace('\r\n','\n')
    return txt

def checkCaption(caption):
    
    # Punctuation not allowed at the end of the short name
    captionTocDisallowedPunctuation = '.!?'
    
    # Missing short name handling
    missing_toc_name_style = 1 # 0 - do not flag, 1 - flag as warning, 2 - flag as error
    
    # Initialize outtxt
    outtxt = ''
    
    # Check for presence of short name, needed for check for citation in TOC regardless
    if '[' not in caption.split('{')[0]: 
        notoc_name = True
        short_name = ' '
    else:
        notoc_name = False
        short_name = caption.split('[')[1].split(']')[0]
    
    if notoc_name and missing_toc_name_style == 1:
        outtxt = outtxt + "WARNING, %s caption, %s does not have a TOC name"%(file,caption) + "\n"
    elif notoc_name and missing_toc_name_style == 2:
        outtxt = outtxt + "ERROR, %s caption, %s does not have a TOC name"%(file,caption) + "\n"
    
    # Check if short name ends in disallowed puncuation
    if short_name[-1] in captionTocDisallowedPunctuation:
        outtxt = outtxt + "WARNING, %s caption, %s TOC name ends in '%s'"%(file,caption, short_name[-1]) + "\n"
    
    # Check if citation is included in the name used in TOC
    if notoc_name and '\\cite' in caption:
        outtxt = outtxt + "ERROR, %s citation in caption, %s"%(file,caption) + "\n"
    elif '\\cite' in short_name:
        outtxt = outtxt + "ERROR, %s citation in caption, %s"%(file,caption) + "\n"
        
    return outtxt

def check_disallowed_commands(txt, file):
    disallowed_commands = [] #['\\bf{','\\tt{']
    outtxt = ''
    for cmd in disallowed_commands:
        split = txt.split(cmd)
        if len(split) > 1:
            for j in range(1, len(split)):
                line_count = len(split[j-1].split('\n'))+1
                outtxt = outtxt + "ERROR, %s %s located at line %d\n"%(file, cmd, line_count)
    return outtxt

args = sys.argv
parser = argparse.ArgumentParser(prog='check_manuals',
                                 description='checks latex manuals for alignment with FDS developer guidelines')
parser.add_argument('call')
parser.add_argument('--file', help='filename to analyze', default='')
parser.add_argument('--datafile', help='filename containing list of files', default='')
parser.add_argument('--globfile', help='glob search for files', default='')
parser.add_argument('--outdir', help='directory to store output', default='')
parser.add_argument('--outname', help='output filename', default='check_output.err')
parser.add_argument('--suppressconsole', help='boolean flag specifying whether findings are printed to console', action='store_true')

cmdargs = parser.parse_args(args)
suppressconsole = cmdargs.suppressconsole
if cmdargs.file != '':
    texfiles = [cmdargs.file]
elif cmdargs.datafile != '':
    with open(cmdargs.datafile, 'r') as f:
        texfiles = f.readlines()
    texfiles = [x.replace('\n','') for x in texfiles]
elif cmdargs.globfile != '':
    texfiles = glob.glob(cmdargs.globfile)
else:
    if not suppressconsole:
        print("Warning, one of --file, --datafile, or --globfile is expected. Checking tex files in current directory")
    texfiles = glob.glob('*.tex')

if not suppressconsole:
    print("Files to check:")
    for file in texfiles:
        print(file)
outdir = cmdargs.outdir
outname = cmdargs.outname

firemodels = os.path.join(os.path.dirname(__file__),'..','..','..')

# This can be called if the white list is being edited and you want sort it
# sortWhitelist(firemodels)

outtxt = '\n'
for i in range(0, len(texfiles)):
    file = texfiles[i]
    
    with open(file, 'r') as f:
        txt = f.read()
    
    # Check figures
    figs = txt.split('begin{figure}')
    for j in range(1, len(figs)):
        fig = figs[j].split('end{figure}')[0]
        captions = fig.split('\\caption')
        for k in range(1, len(captions)):
            caption = captions[k].split('}')[0] + '}'
            outtxt = outtxt + checkCaption(caption)
            
    # Check tables
    tabs = txt.split('begin{table}')
    for j in range(1, len(tabs)):
        tab = tabs[j].split('end{table}')[0]
        captions = tab.split('\\caption')
        for k in range(1, len(captions)):
            caption = captions[k].split('}')[0]
            outtxt = outtxt + checkCaption(caption)
    
    # Check disallowed commands
    outtxt = outtxt + check_disallowed_commands(txt, file)
    
    # Check spelling
    txt = checkSpelling(file, firemodels)
    
    if len(txt) > 0:
        txt_list = list(set(txt.split('\n')))
        txt_list.sort()
        #while '\n\n' in txt:
        #    txt = txt.replace('\n\n','\n')
        #outtxt = outtxt + '\n\nMisspelt Words in %s:\n'%(file) + '\n'.join(txt_list) + '\n\n'
        for j in range(0, len(txt_list)):
            outtxt = outtxt + '\n\nMisspelt Words in %s: %s\n'%(file,txt_list[j])

if len(outtxt) > 1 and not suppressconsole:
    print("Warnings identified in the manual check:")
    print(outtxt)

if outdir != '': outname = outdir + os.sep + outname
with open(outname, 'w') as f:
    f.write(outtxt)
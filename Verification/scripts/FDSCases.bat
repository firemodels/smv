@echo off
set exe=%1
set option=%2
setlocal

set allfinished=1

cd ..\..\..\fds\Verification
set verdir=%CD%

cd ..\..\smv\Verification
set smvverdir=%CD%
cd scripts\bin
set GREP=%CD%\grep.exe
set WC=%CD%\wc.exe

cd %verdir%

set run=call :runit %exe% %option%


%run%  Adaptive_Mesh_Refinement random_meshes
%run%  Aerosols propane_flame_deposition
%run%  Complex_Geometry geom_simple
%run%  Complex_Geometry geom_obst
%run%  Complex_Geometry geom_sphere1a
%run%  Complex_Geometry geom_sphere1c
%run%  Complex_Geometry geom_sphere1e
%run%  Complex_Geometry geom_sphere3b
%run%  Complex_Geometry geom_sphere3d
%run%  Complex_Geometry geom_sphere3f
%run%  Complex_Geometry geom_sphere1b
%run%  Complex_Geometry geom_sphere1d
%run%  Complex_Geometry geom_sphere1f
%run%  Complex_Geometry geom_sphere3a
%run%  Complex_Geometry geom_sphere3c
%run%  Complex_Geometry geom_sphere3e
%run%  Complex_Geometry geom_terrain
%run%  Complex_Geometry geom_texture
%run%  Complex_Geometry geom_texture2
%run%  Complex_Geometry geom_texture3a
%run%  Complex_Geometry geom_texture3b
%run%  Complex_Geometry geom_texture4a
%run%  Complex_Geometry geom_texture4b
%run%  Complex_Geometry sphere_radiate
%run%  Complex_Geometry cone_1mesh
%run%  Complex_Geometry t34_scaling
%run%  Controls activate_vents
%run%  Detectors beam_detector
%run%  Detectors objects_static
%run%  Detectors objects_dynamic
%run%  Flowfields symmetry_test
%run%  Flowfields symmetry_test_2
%run%  Flowfields symmetry_test_mpi
%run%  Flowfields jet_fan
%run%  Heat_Transfer ht3d_ibeam
%run%  HVAC HVAC_mass_conservation
%run%  HVAC HVAC_energy_pressure
%run%  HVAC leak_test_2
%run%  Miscellaneous pyramid
%run%  Miscellaneous obst_multi
%run%  Miscellaneous obst_sphere
%run%  Miscellaneous obst_cylinder
%run%  Miscellaneous obst_cone
%run%  Miscellaneous obst_rotbox
%run%  NS_Analytical_Solution ns2d_64
%run%  Pressure_Solver dancing_eddies_embed
%run%  Pressure_Solver dancing_eddies_tight
%run%  Scalar_Analytical_Solution move_slug
%run%  Scalar_Analytical_Solution move_slug_fl1
%run%  Scalar_Analytical_Solution shunn3_256
%run%  Scalar_Analytical_Solution soborot_charm_square_wave_64
%run%  Sprinklers_and_Sprays cascade
%run%  Turbulence csmag_64
%run%  Turbulence dsmag_64
%run%  WUI Bova_1a
%run%  WUI Bova_1b
%run%  WUI Bova_4a
%run%  WUI level_set_fuel_model_1
if %exe% == check if %allfinished% == 1 echo all cases finished
goto eof

:runit
  set prog=%1
  set option=%2
  set casedir=%3
  set input=%4

  cd %verdir%\%casedir%
  if %prog% == check goto skip1

  if %option% == fds timeout /t 2 /nobreak & start "%input%" cmd /c "%smvverdir%\scripts\background.bat %prog% %input%.fds"
  if %option% == smv %prog% -runscript %input%
  exit /b

:skip1
  set finished=0
  if exist %input%.out %GREP% -E only^|success %input%.out | %WC% -l > %input%.wc
  set /p finished=<%input%.wc
  if %finished% == 0 echo %input% did not finish
  if %finished% == 0 set allfinished=0
  exit /b

:eof 

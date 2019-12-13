# voxelizeAlgorithm

Using a windows forms applicaiton as a user interface, this program inputs .obj and .ply files of a CAD model and 
outputs a voxelized approximation of the model. I made this to be used for MCNP simulations so the output has some 
MCNP input card stuff in it also. The voxel cell sizes are normalized to the size of whatever units you used to make 
the CAD model. So, if you want a better approximation, make the units in CAD smaller.

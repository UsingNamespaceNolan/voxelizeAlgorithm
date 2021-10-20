# voxelizeAlgorithm

Using a C++/CLR Windows Form as a user interface, this program takes .obj and .ply files of a CAD model as input and outputs a file representing a voxelized approximation of the model to be used in MCNP simulations. The voxel cell sizes are normalized to the size of whatever units you used to make the CAD model. So, if you want a better approximation, make the units in your CAD software smaller.

# How to model vehicles

This tutorial provides some outlines for modelling vehicles, more specifically, 4-wheeled vehicles. 

* [__Modelling__](#modelling)  
* [__Textures__](#textures)  
* [__RIG__](#rig)  
* [__LODs__](#lods)  

---
## Modelling

Vehicles must have a minimum of 10.000 and a maximum of 17.000 Tris approximately. CARLA vehicles are created using the size and scale of real cars.
The bottom part of the vehicle consists of a plane adjusted to the Bodywork.

The vehicle must be divided in 6 materials.  

*   __BodyWork.__ Includes the chassis, doors, car handle, and front and back parts of the vehicle. This material is controlled by Unreal Engine. Logos and some details can be added, but all the details will be painted by UE using the same color. Use the alpha channel to paint details with a different color.  
	*   __Name of the material.__ `M_<CarName>_Bodywork<PartName>`  
<br>
*   __Wheels.__ Model these with *hubcaps* and add details to the tire with *Substance*. In the UV, add the tires and the hubcaps separately.  
	*   __Name of the material.__ `M_<CarName>_Wheel`  
<br>
*   __Interior.__ Includes the seats, the steering wheel, and the bottom of the vehicle. There is no need to add much detail here.  
	*   __Name of the material.__ `M_<CarName>_Interior`  
<br>
*   __Details.__ Lights, logos, exhaust pipes, protections, and grille.  
	*   __Name of the material.__ `M_<CarName>_Details`  
<br>
*   __Glass.__ Light glasses, windows, etc. This material is controlled by Unreal Engine.  
	*   __Name of the material.__ `M_<CarName>_Glass`  
<br>
*   __LicensePlate.__ Us a rectangular plane of 29x12cm for the license Plate. Assign the license plate texture to it.  
	*   __Name of the material.__ `M_<CarName>_LicensePlate`  

---
## Textures

The size of the textures is 2048x2048.

* T_"CarName"_PartOfMaterial_d (BaseColor)

* T_"CarName"_PartOfMaterial_n (Normal)

* T_"CarName"_PartOfMaterial_orm (OcclusionRoughnessMetallic)


For instance, in the __Tesla Model 3__, there is one material with several textures.  

*   __Material.__ `M_Tesla3_BodyWork`
	*   __Textures.__ `T_Tesla3_BodyWork_d`, `T_Tesla3_BodyWork_n`, `T_Tesla3_BodyWork_orm`

---
## RIG

The simplest way is to reuse the `General4WheeledVehicleSkeleton` in CARLA. Either export and copy it to the new model, or create the skeleton using the same bone names and orientation.

!!! Important
    The model and every bone must be oriented towards __positive X axis__ with the __Z axis facing upwards__.

__Bone setup.__  

*   __Vehicle_Base.__ The origin point of the mesh, place it in the point (0,0,0) of the scene.

*   __Wheel_Front_Left.__ Set the joint's position in the middle of the wheel.

*   __Wheel_Front_Right.__ Set the joint's position in the middle of the wheel.

*   __Wheel_Rear_Left.__ Set the joint's position in the middle of the wheel.

*   __Wheel_Rear_Left.__ Set the joint's position in the middle of the wheel.

---
## LODs

All vehicle LODs must be made in Maya or other 3D software. Because Unreal does not generate LODs automatically. Adjust the number of Tris to make smooth transitions between levels.

* _Level 0_ - Original

* _Level 1_ - Deleted 2.000/2.500 Tris (_Do not delete the interior and steering wheel_)

* _Level 2_ - Deleted 2.000/2.500 Tris (_Do not delete the interior_)

* _Level 3_ - Deleted 2.000/2.500 Tris (_Delete the interior_)

* _Level 4_ - Simple shape of a vehicle.

---

That concludes some basic outlines to model a CARLA vehicle. 

If any doubts arise during the process, the forum is the best place to post them. 

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</p>
</div>


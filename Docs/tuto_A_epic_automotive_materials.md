# How to link Epic's Automotive Materials

Epic Game's provides a set of realistic _Automotive Materials_ for free that were used in previous CARLA releases. This tutorial explains how to download these materials and apply them in CARLA.  

* [__Download the materials__](#download-the-materials)  
* [__Link the materials__](#link-the-materials)  

!!! important
    CARLA does not use Epic's *Automotive Materials* by default since 0.8.0. However, these can be enabled if working from source.

---
## Download the materials

The *Automotive Materials* package can be downloaded from the Unreal Engine Marketplace [__here__][automatlink].

[automatlink]: https://www.unrealengine.com/marketplace/automotive-material-pack

  1. Install Epic Games Launcher from [www.unrealengine.com](https://www.unrealengine.com).
  2. Buy the [Automotive Materials][automatlink] package for $0.
  3. Create a new dummy project and add the Automotive Materials package to it.
  4. Inside the `Content` folder of the newly created project, there will be a `AutomotiveMaterials` folder. Copy this folder to the CARLA project
    - `{NewProject}/Content/AutomotiveMaterials` --> `{CARLA}/Unreal/CarlaUE4/Content/AutomotiveMaterials`

!!! Note
    Linux based machines don't have official support for the Marketplace. In order to gain access to it, download [this Java Package](https://github.com/neutrino-steak/UE4LinuxLauncher).

---
## Manually link the materials

The *Automotive Materials* have to be manually linked inside the project.  

__1.__ In the content browser, go to `Content/Static/Vehicles/GeneralMaterials/MaterialRedirectors`.

__2.__ Open `RedirectorInstance`.

![Unreal Engine Screenshot 1](img/materials_screenshot_00.png)

__3.__ Under the `Details` panel, search for the `Parent` material.

__4.__ Replace the `DummyCar` material by `M_Carpaint` material.

![Unreal Engine Screenshot 2](img/materials_screenshot_01.png)

__5.__ Save `RedirectorInstance`, and the material will be ready to go.


---

That sums up the process to download and apply Epic's *Automotive Materials* in CARLA.  

If any doubts arise during the process, the forum is the best place to post them. 

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</p>
</div>


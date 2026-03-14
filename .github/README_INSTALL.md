<div align="center" markdown="1">

If you want to support my work:

<a href="https://patreon.com/xoxor4d"><img src=".github/img/patreon.png" width="12%"></a>  
<a href="https://ko-fi.com/xoxor4d"><img src=".github/img/kofi.png" width="10%"></a>

</div>

<br>

------

# Installation:

### Install using the installer:
1. Download <LINK_TO_MOD_ZIP>
2. Download <LINK_TO_INSTALLER>

3. Place both files in the same folder (_no need to copy them to your game folder_) and run `Portal2-Remix-CompMod-Installer.exe` 
4. Use the File Dialog to select your `portal2.exe` which is located in your Portal2 install folder
5. Make sure that you remove all custom launch arguments for Portal2 in Steam (if you have any)

<br>

### OR Install manually (no need to do this if you've used the installer):
1. Download <LINK_TO_MOD_ZIP>

2. Open the zip and extract all files contained inside the `Portal2-Remix-CompatibilityMod` folder into your Portal2 directory (next to the `portal2.exe`). Overwrite all when prompted.

3. Download the [`base remix-mod`](https://github.com/xoxor4d/p2-rtx-base-mod/archive/refs/heads/master.zip) - repo: (https://github.com/xoxor4d/p2-rtx-base-mod)
4. Extract and place the `mods` folder __into__ the `rtx-remix` folder found in the Portal 2 __root__ folder  
(create the `rtx-remix` folder if it does not exist) (see [this](#folder-structure))

> [!Note]
> Only maps up until Act 2 (catapult intro) are touched up. Other maps might look weird / dark. That also includes water surfaces.

<br>

# Usage and general Info
- Run the game via the provided `run-p2-rtx.bat` batch file or copy and paste the following into the steam launch args for Portal 2:  
`-insecure -steam -novid -disable_d3d9_hacks -limitvsconst -softparticlesdefaultoff -disallowhwmorph -no_compressed_verts -nogamepadui +mat_phong 1`

- You should hear a beep followed by a green banner with a countdown at the top left corner of your window
  > Press Alt + X to open the Remix menu  
  > Press F6 to open the Compatibility Mod menu

<br>

> [!Note]
> portal2.exe was modified to load p2-rtx.dll to allow launching the mod via steam  
> because asiloader does not get loaded when launched from steam.

<br>

> [!Important]
> **Usage Info / Guides** -- Look into the **Wiki**:  
> https://github.com/xoxor4d/p2-rtx/wiki  

<br>

## <a id="folder-structure"></a>Folder Structure
</div>

```
.  
├─ ...
├─ 📁 steamapps
│  └─📁 common
│     └─📁 Portal 2
│       ├── 📜 run-p2-rtx.bat
│       ├── 📜 portal2.exe
│       ├── 📜 toggle-p2-rtx.bat
│       ├── 📜 p2-rtx.dll
│       ├── 📜 ...
│       │
│       ├── 📁 bin
│       │   └─📁 .trex
│       │
│       ├── 📁 portal2_dlc2
│       ├── 📁 portal2_dlc3
│       ├── 📁 ...
│       └── 📁 rtx-remix
│           └─📁 mods
│             └─📁 portal2rtx
│               ├── 📜 _compatibility.usda
│               ├── 📜 _highquality.usda
│               └── ...
└── ...  
```

<br>
<br>

----

## ✳️ Info:
- Current releases ship with a custom build of the [remix-dxvk runtime](https://github.com/xoxor4d/dxvk-remix/tree/game/p2_rebase2) including necessary changes for Portal 2
- Some engine tweaks required to make the game compatible with RTX Remix result in CPU bottlenecks on some maps (software skinning instead of HW skinning, drawcall amount). This may or may not improve in future updates.
- There are clever ways to optimize many aspects of this game but that takes time and lots of effort.
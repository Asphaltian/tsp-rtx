# Installation:

### Install using the installer:
1. Download <LINK_TO_MOD_ZIP>
2. Download <LINK_TO_INSTALLER>

3. Place both files in the same folder (_no need to copy them to your game folder_) and run `TheStanleyParable-Remix-CompMod-Installer.exe` 
4. Use the File Dialog to select your `stanley.exe` which is located in your The Stanley Parable install folder
5. Make sure that you remove all custom launch arguments for The Stanley Parable in Steam (if you have any)

<br>

### OR Install manually (no need to do this if you've used the installer):
1. Download <LINK_TO_MOD_ZIP>

2. Open the zip and extract all files contained inside the `TheStanleyParable-Remix-CompatibilityMod` folder into your The Stanley Parable directory (next to the `stanley.exe`). Overwrite all when prompted.

3. Download the [`base remix-mod`](https://github.com/Asphaltian/tsp-rtx-base-mod/archive/refs/heads/master.zip) - repo: (https://github.com/Asphaltian/tsp-rtx-base-mod)
4. Extract and place the `mods` folder __into__ the `rtx-remix` folder found in The Stanley Parable __root__ folder  
(create the `rtx-remix` folder if it does not exist) (see [this](#folder-structure))

> [!Note]
> Only some maps are touched up. Other maps might look weird / dark.

<br>

# Usage and general Info
- Run the game via the provided `run-tsp-rtx.bat` batch file or copy and paste the following into the steam launch args for The Stanley Parable:  
`-insecure -steam -novid -disable_d3d9_hacks -limitvsconst -softparticlesdefaultoff -disallowhwmorph -no_compressed_verts -console +mat_phong 1`

- You should hear a beep followed by a green banner with a countdown at the top left corner of your window
  > Press Alt + X to open the Remix menu  
  > Press F6 to open the Compatibility Mod menu

<br>

> [!Note]
> stanley.exe was modified to load tsp-rtx.dll to allow launching the mod via steam  
> because asiloader does not get loaded when launched from steam.

<br>

> [!Important]
> **Usage Info / Guides** -- Look into the **Wiki**:  
> https://github.com/Asphaltian/tsp-rtx/wiki  

<br>

## <a id="folder-structure"></a>Folder Structure
</div>

```
.  
├─ ...
├─ 📁 steamapps
│  └─📁 common
│     └─📁 The Stanley Parable
│       ├── 📜 run-tsp-rtx.bat
│       ├── 📜 stanley.exe
│       ├── 📜 toggle-tsp-rtx.bat
│       ├── 📜 tsp-rtx.dll
│       ├── 📜 ...
│       │
│       ├── 📁 bin
│       │   └─📁 .trex
│       │
│       ├── 📁 ...
│       └── 📁 rtx-remix
│           └─📁 mods
│             └─📁 thestanleyparablertx
│               ├── 📜 _compatibility.usda
│               ├── 📜 _highquality.usda
│               └── ...
└── ...  
```

<br>
<br>

----

## ✳️ Info:
- Current releases ship with a custom build of the [remix-dxvk runtime](https://github.com/xoxor4d/dxvk-remix/tree/game/p2_rebase2) including necessary changes for The Stanley Parable
- Some engine tweaks required to make the game compatible with RTX Remix result in CPU bottlenecks on some maps (software skinning instead of HW skinning, drawcall amount). This may or may not improve in future updates.
- There are clever ways to optimize many aspects of this game but that takes time and lots of effort.

# gd-yt

A simple Geometry Dash mod that lets you use YouTube songs in custom levels


## Installation

First you'll need to create some additional directories in your Geometry Dash folder

```
.
└── Geometry Dash/
    ├── gd-yt/
    │   └── downloads/
    └── ...
```

Then download [yt-dlp](https://github.com/yt-dlp/yt-dlp/releases/tag/2023.03.04) and paste the executable in 'gd-yt' directory. Your final file structure should look like this:

```
.
└── Geometry Dash/
    ├── gd-yt/
    │   ├── downloads/
    │   └── yt-dlp.exe
    └── ...
```

For this mod to work you'll need 'Change custom songs location' option to be turned on (to be fixed)

Finally download DLL files from [here](https://github.com/bartoszstepien01/gd-yt/releases). Move 'minhook.x32.dll' to the GD root folder. Then inject 'gd-yt.dll' using a DLL injector like [this](https://github.com/adafcaefc/ProxyDllLoader)
    
## Usage

In order to use this mod go to song selection menu in the editor. You should see a checkbox next to the ID input. After clicking it and searching for a YouTube video ID you should be able to see the song details below. 

Sometimes you'll have to restart the editor and go back to 'My Levels' menu for it to actually work.


## TODO
- Simplify installation process
- Fix visual bugs
- Remove game freezes when fetching data from yt-dlp
- Hide video ID from description
- Remove the need of turning 'Change custom songs location' on
## Acknowledgements

- HJFod for GD headers
- nlohmann for json library
- TsudaKageyu for MinHook library
- tomykaira for Base64 headers
- matcool for MappedHook
- poweredbypie for cocos2d headers


## License

[MIT](https://choosealicense.com/licenses/mit/)


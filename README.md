# REngine

A real-time 3D scene viewer written from scratch in C++17 and OpenGL 3.3, built around a small component-based scene graph. It renders an interactive outdoor scene with a day/night cycle, animated water and vegetation, mouse picking, spline-based camera and object animation, and a TV that plays real video files through libVLC.

Originally developed as a semester project for the Computer Graphics (PGR) course at CTU FEE.

## Features

**Rendering**
- Blinn-Phong shading with directional, point and spot lights (up to 100), uploaded through uniform buffer objects
- Material system with up to 16 textures per material, normal mapping (TBN), emission and shininess
- Distance fog, optionally sampled from the skybox cubemap so fog matches the sky
- Day/night cycle with blended day and night skyboxes and a moving sun
- MSAA, back-face culling, off-screen framebuffers, runtime shader switching

**Scene system**
- `SceneManager` / `SceneObject` with attachable components: `TransformComponent`, `ModelComponent`, `CameraComponent`, `LightComponent`
- `ObjectScript` base class for per-object behaviour (camera, skybox, TV, biplane, outline, text)
- Model loading via Assimp (`.obj`, `.glb`), texture loading via stb_image, resource caching in `ResourceManager`

**Interaction**
- Mouse picking: objects are rendered to a separate framebuffer with UUID-encoded colours, so the object under the cursor is read back per pixel
- Selection outline shader
- Configurable key bindings loaded from a text file, including multi-key combinations
- ImGui debug overlay with a scene inspector and draw-call / visible-object counters

**Extras**
- Animated water surface and wind-driven vegetation in the vertex shaders
- Cubic NURBS / B-spline curves (Cox–de Boor) used for the flying biplane and for camera fly-through paths
- Text rendering with FreeType
- Video playback on the TV screen through libVLC, with in-scene play / next / previous buttons

## Project structure

```
core/             renderer, buffers, shaders, textures, scene graph, curves, input, time
components/       transform, model, camera, light
ObjectScripts/    per-object behaviour (camera, skybox, TV, film, biplane, text, outline)
gui/              ImGui debug panel
shaders/          GLSL vertex and fragment shaders
resources/        models, textures, skyboxes, fonts, videos
main.cpp          entry point and main loop
setup.cpp/.h      scene construction and config loading
```

## Dependencies

Fetched automatically by CMake (`FetchContent`): GLAD, GLFW 3.4, GLM, Dear ImGui, Assimp, FreeType, stb, stduuid.

libVLC is the only dependency you provide yourself on Linux; on Windows the SDK is downloaded automatically.

## Building

**Linux** — install the system packages first:

```bash
sudo apt install build-essential cmake libgl-dev libvlc-dev vlc \
                 vlc-plugin-base vlc-plugin-video-output
```

**Build (all platforms):**

```bash
cmake -B build
cmake --build build --config Release
```

The executable `REngine` is placed in the project root, since resources are loaded through relative paths — run it from there:

```bash
./REngine
```

Prebuilt Linux binaries are published under [Releases](../../releases).

## Controls

| Action | Binding |
| --- | --- |
| Interact / select object | Left mouse button |
| Look around | Hold right mouse button |
| Move | `W` `A` `S` `D` |
| Up / down | `Q` / `E` |
| Sprint | `Left Shift` |
| Move camera to next curve point | `M` + `N` |
| Stop moving along curve | `P` |
| Toggle debug overlay | `L` |
| Show picking (UUID) buffer | `U` |
| Reload settings from disk | `R` |

## Configuration

All three files are plain text and are read at startup; `R` reloads them at runtime.

- `keybinds.txt` — action bindings in the form `ActionName DEVICE KEYCODE [DEVICE KEYCODE ...]`, using [GLFW key](https://www.glfw.org/docs/3.3/group__keys.html) and [mouse button](https://www.glfw.org/docs/3.3/group__buttons.html) codes. Listing several tokens makes the action require all of them at once.
- `nature_settings.txt` — wave height and speed, wind strength, speed and direction, and the length of a full day in seconds.
- `tv_videos.txt` — one video path per line; these become the TV's playlist.

## Continuous integration

- `cmake.yml` builds the project on Ubuntu for every push and pull request to `main`.
- `build_release.yml` builds, packages the binary together with `shaders/`, `resources/` and the config files, and publishes a GitHub Release when a `v*` tag is pushed.

## License

Not licensed yet. Third-party assets (models, textures, fonts, videos) under `resources/` are subject to their own licenses.

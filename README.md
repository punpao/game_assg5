---

## Features

### Core
- Third-person, orbit camera (mouse drag to rotate, scroll to zoom)
- Skeletal animation (GPU skinning)
- Real-time bone transform updates on CPU
- Supports external 3D model + animation (Assimp .dae / .fbx / .obj)
- Works with bone-weighted meshes (up to 200 bones)

### Rendering
- Custom orbit camera (does not depend on LearnOpenGL Camera class)
- Depth testing enabled
- Per-vertex bone influences and weights
- Bone matrix array sent to vertex shader

---

## Controls

| Key / Mouse Action | Description |
|--------------------|-------------|
| W / S              | Move character forward / backward |
| A / D              | Rotate character |
| Left Mouse Drag    | Rotate orbit camera around character |
| Scroll Wheel       | Zoom camera in/out |
| Space              | Pause / Resume animation |
| 1 / 2 / 3          | Change animation speed (slow / normal / fast) |
| R                  | Reverse playback direction |
| ESC                | Quit program |

---

[demo.mp4](https://punpao.github.io/game_assg5/demo.mp4)

# Embedded assets

The game keeps its media in compiled C arrays so that the target does not need a file system.

| File | Contents | Consumer |
| --- | --- | --- |
| `assets/assets_gun.c` | 256 × 128 RGB565 gun image with a transparent key | `src/render.c` |
| `assets/assets_sound.c` | Fire and target-hit sample arrays | `src/audio.c` |
| `src/cover.c` | 320 × 240 RGB565 title image and its VGA draw routine | `src/main.c` |
| `docs/images/title-screen.png` | PNG preview decoded from `title_img` for repository presentation | `README.md` |

`docs/images/title-screen.png` is documentation only and is not loaded by the game. The runtime continues to use the array compiled into `src/cover.c`.

The original source snapshot did not include separate provenance or license metadata for the embedded image and audio data. No new license is assigned to those assets by the repository-organization changes.

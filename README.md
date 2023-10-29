# engy

WIP gamedev stuff

## Building

Requires a CPU that supports Intel SSE4.1 instructions.

Prerequisites:

- SDL2
- OpenGL

Once prerequisites are installed, compile it:

```bash
meson setup builddir
cd builddir
ninja
```

To run, you need to set `ASSET_PATH` as an environment variable. Your
`ASSET_PATH` needs to include the `shaders/` directory from this repo as
the shaders aren't built into the binary yet (TBD in the future). On
Linux, you can symlink the shaders into your asset folder.

```bash
mkdir $HOME/assets
cd $HOME/assets
ln -s $HOME/[path to repo]/assets
export ASSET_PATH=$HOME/assets
```

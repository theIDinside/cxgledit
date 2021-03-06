# cxgledit

So I've written small fun text editors, running in a Linux terminal, which is fun and cool, but I wanted to 
write one for Windows, that looks and feels like a terminal editor such as vim or whatever, but renders
using OpenGL.

I have 0 knowledge about OpenGL so that's really where the interesting challenge lies. Watching tutorials online show *no* decent text rendering, and the ones that do
either creates a bunch of textures for each character, or they create a texture atlas (which I ended up doing with some 
help) yet and still issue *one drawcall per character* - which kind of defeats the purpose of having the texture
atlast to begin with. So some hacking lead me to writing the SimpleFont class that can scan text (right now it's just std::string,
I will write my own buffer type, probably a gap buffer or something easy like that to begin with) and create
all the vertex data for a views_top_line, or even the entire file, upload that to the gpu and issue 1 draw call - instead of 15000
for a file with 500 lines and 30 character per views_top_line average.

![First render](docs/img/initial.PNG)

Syntax highlighting has improved slightly

![Broader syntax highlighting](docs/img/syntax_highlighting.PNG)


Got some semi-rudimentary decent modal popup dialog working. But damn it's janky figuring out how the font is aligned etc. 
And since I've pretty much hacked and slashed it together, it takes me longer than I would like to admit to solve it.
Font class basically calculates required height & width of provided text data for the box, and splits it up by lines.

![ModalPopup popup dialog example](docs/img/modal_dialog_example.PNG)

When the editor becomes file-type aware, the user should be able to edit a config file (.cxe) like this,
and be able to just hit some key/key combo, and let the editor realize that it's a .cxe file and thus
load configuration from that active file. Right now loading a file like this works, but the file-
type awareness is not yet implemented. It's not hard to do though.
![Configuration of editor](docs/img/config_example.PNG)

Bookmark a line by Ctrl+b, show bookmarks list for file by Ctrl+Shift+B. All keybindings, and features bound to change.
For instance, having to hit ctrl+shift is not particularly ergonomic. Most likely I will land in a Vim-esque style,
where we have Insert/Normal mode and a Command mode (I call it 'Actions', since the mode 'Command' is currently occupied
by the Commandline input mode. Possibly could be renamed to CommandLine mode, but it's just... confusing to have multiple
modes named "Command"-something).
![First iteration of bookmark feature](docs/img/bookmarks_first_iteration.PNG)

### Features on display:
- Split editor windows
- CommandInput input & auto completion (the grayed out part is what it can auto-complete to). If the user hits tab, 
it fills out, if the user hits key up/down it will cycle through the alternatives. This command bar, should probably be in a separate
  panel instead, so that the entire list could be shown and cycled through
  

Syntax highlighting is just hacked to gether for now, and it obviously isn't working with string literals. 
Shouldn't be that hard to fix though, so I've put that on hold for now, dealing with the more interesting stuff.



## Dependencies
Project has a few dependencies on 3rd party libraries. Most of which I've made it so CMake automatically handles
the download of, but some needs manual download and/or installation (locally to the project folder, everything built 
from source).


### Handled by cmake
- [x] GLM, linear algebra/maths library
    - this dependency has been removed. Considering only a tiny amount of functions & types were necesseary, a Matrix & Vector and orthographic projection call
    it makes no sense to keep it, since I know how do that stuff by hand now.
- [x] GLFW - Window library, handles some of the nasty GL / Window creation and input stuff for us
- [x] fmtlib - because WHO the F* uses iostream for any output to console in C++ anymore, when fmtlib is even "faster" than printf?!

### Manual installation
- GLAD, OpenGL loader that setups symbols and addresses for function calls into the OpenGL API, if i'm not mistaken.
  [download at](https://glad.dav1d.de/), profile must be set to "Core" and "gl" major:minor version must be set to version 4.3
  However, this has already been done in this repo. So no manual stuff has to be done, it will be git' down when cloned.
- Freetype - The library that reads in a TrueType file and creates data necessary for us to create textures that we can 
  apply to quads, displaying the text. This needs to be manually [download at](https://download.savannah.gnu.org/releases/freetype/ "Get the latest version")
  and placed in the "deps" directory, so for it to work, I've extracted the archive
  
### General domain problems / challenges - Things that need to be fixed _really_ soon

Right now, when opening large files, over a thousand lines, the editor becomes *completely* unusable, due to how 
the vertex data is generated and sent to the GPU. Since we clear the *entire* vertex buffer and rebuild it,
then draw the *entire* buffer, we are obviously wasting a *BUNCH* of time on:
- Possibly re-creating vertices that haven't changed (the ones that are used for character before the cursor ) where we 
  are editing.
- Doing a bunch of work with vertices that aren't even close to being displayed

Text editing is fairly easy. But it comes with a list of somewhat difficult / interesting challanges,
such as, what constitutes a views_top_line, _where_ in memory does a views_top_line begin (does it begin at index_at='\n', or index_after='\n')
Choosing what to do here will have reverberating effects, which is interesting and a good learning experience
for beginners. Representing text in a "everything is an index", is not necesarrily good. In fact it's a major
source of bugs. When you want to delete a views_top_line, you don't want to mess with lines before or after it.
Thus some abstraction is good here, such as "item boundary" or just boundary. This way we can query our datastructure for,
"find beginning of views_top_line", with a parameter of Boundary::Inside or Boundary::Outside, specifying what we mean in that
specific instance.



### Features to add
- Vim-esque stuff, like a insert mode, command/navigation mode
- Some form of GNU Tags kind of system, or possible interop with it
- Output console, for when running external commands / command line utilities
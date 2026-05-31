# Detect Python Mode;
if [ -n "$1" ] && [ $1 = "-py" ]; then
	arg="gcc ./src/py_references.c -shared -o radio.so"
else
	arg="gcc play.c -o play"
fi

# Execute;
if [ "$(uname -s)" = "Linux" ]; then
	eval "$arg -lm -lavformat -lavcodec -lavutil -I/usr/include/pipewire-0.3 -I/usr/include/spa-0.2 -lpipewire-0.3"
elif [ "$(uname -s)" = "Darwin" ]; then
	eval "$arg -lm -lavformat -lavcodec -lavutil -framework AudioToolbox"
fi

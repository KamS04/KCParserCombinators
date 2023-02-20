WDIR=$(pwd)
lin="$WDIR/build $WDIR/dist/parselib-linux/libparselib.a"
linDebug="$WDIR/dbuild $WDIR/dist/parselib-linux/libdparselib.a"
eval $lin
eval $linDebug
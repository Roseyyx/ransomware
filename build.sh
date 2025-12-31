set -eu
script_path="$(realpath "$0")"
root="$(dirname "$script_path")"

pushd "$(dirname "$0")"

compiler=${CC:-gcc}

src_files=(
  src/main.c
)

mkdir -p build
pushd build >>/dev/null

all_src=""
for p in "${src_files[@]}"; do
  all_src+=" ../${p}"
done

common_opts="-I$root/src -Wall"
debug_opts="--debug -g --optimize -O2 -Werror -DDEBUG $common_opts"

compile="$compiler $all_src -o ransomware $debug_opts $(pkg-config --cflags --libs gtk+-3.0 jansson) -lcurl "

echo "$compile"
$compile

popd >>/dev/null
popd >>/dev/null

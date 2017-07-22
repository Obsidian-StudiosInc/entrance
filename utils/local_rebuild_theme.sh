#!/bin/bash

EDJ="data/themes/default/default.edj"

[[ -f "${EDJ}" ]] && rm "${EDJ}"

make "${EDJ}"

[[ -f "${EDJ}" ]] && cp "${EDJ}" test/entrance/themes

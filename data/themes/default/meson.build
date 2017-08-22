cmd = [ edje_cmd,
        '-id', join_paths(meson.source_root(), 'data' , 'themes', 'default', 'images'),
        '@INPUT@', '@OUTPUT@'
      ]
src = 'default'
custom_target(src + '.edj',
              input      : src + '.edc',
              output     : src + '.edj',
              command    : cmd,
              install_dir: join_paths(dir_data, proj, 'themes'),
              install    : true
             )
out = join_paths(dir_data, proj, 'themes', 'default.edj')
meson.add_install_script(chmod, 'a+r', out)

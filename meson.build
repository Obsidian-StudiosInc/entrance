# project
project('entrance', 'c',
        version        : '0.1.0_alpha1',
        license        : 'GPL-3.0',
        default_options: [ 'c_std=gnu99' ],
        meson_version  : '>= 0.40.0')
base_url = 'https://github.com/Obsidian-StudiosInc/'

# global variables
proj     = meson.project_name()
ver      = meson.project_version()
chmod    = join_paths(meson.source_root(), 'meson-chmod.sh')

# dependencies
efl_version = '>= 1.19.0'
deps	= dependency('elementary', version: efl_version)
ecore_x	= dependency('ecore-x',	   version: efl_version)
edje	= dependency('edje',       version: efl_version)
xcb	= dependency('xcb')

has_pam = get_option('pam')
has_ck  = get_option('consolekit')
cc = meson.get_compiler('c')

if has_pam
        pdeps = cc.find_library('pam')
        pinc  = include_directories('/usr/include/security')
else
        pdeps = cc.find_library('crypt')
	#FIXME
        pinc  = include_directories('/usr/include')
endif

##### edje_cc binary compiler tool
edje_cmd = get_option('edje-cc')
if edje_cmd == ''
  edje_cmd = join_paths(edje.get_pkgconfig_variable('prefix'),
                        'bin', 'edje_cc')
endif

##### dir locations
dir_prefix = get_option('prefix')
dir_bin    = join_paths(dir_prefix, get_option('bindir'))
dir_lib    = join_paths(dir_prefix, get_option('libdir'))
dir_data   = join_paths(dir_prefix, get_option('datadir'))
dir_sbin   = join_paths(dir_prefix, get_option('sbindir'))
dir_syscfg = join_paths(dir_prefix, get_option('sysconfdir'))

##### config.h
cfg = configuration_data()
if has_ck
	cfg.set('HAVE_CONSOLEKIT', true)
endif
if has_pam
	cfg.set('HAVE_PAM', true)
endif
cfg.set_quoted('PACKAGE'		, proj)
cfg.set_quoted('PACKAGE_NAME'		, proj)
cfg.set_quoted('PACKAGE_STRING'		, proj + ' ' + ver)
cfg.set_quoted('PACKAGE_URL'		, base_url + proj)
cfg.set_quoted('PACKAGE_BIN_DIR'	, join_paths(dir_bin, proj))
cfg.set_quoted('PACKAGE_DATA_DIR'	, join_paths(dir_data, proj))
cfg.set_quoted('PACKAGE_LIB_DIR'	, dir_lib)
cfg.set_quoted('PACKAGE_SBIN_DIR'	, dir_sbin)
cfg.set_quoted('SYSTEM_CONFIG_DIR'	, dir_syscfg)
cfg.set_quoted('VERSION'		, ver)

configure_file(input  : 'data/entrance.conf.in',
               output : 'entrance.conf',
               configuration : cfg
)
configure_file(input  : 'data/entrance.service.in',
               output : 'entrance.service',
               configuration : cfg
)
configure_file(output: 'config.h', configuration: cfg)
add_project_arguments('-include', 'config.h', language : 'c')

##### subdirs
subdir('src/bin')
subdir('src/daemon')
subdir('data/themes/default')

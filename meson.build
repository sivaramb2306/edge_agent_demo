project('edge_agent', 'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++17'])

# Dependencies
pistache_dep = dependency('libpistache')
json_dep = dependency('nlohmann_json')
netsnmp_dep = dependency('netsnmp')

# Main executable
edge_agent = executable('edge-agent',
  'main.cpp',
  dependencies : [
    pistache_dep,
    json_dep,
    netsnmp_dep
  ],
  include_directories : [
    include_directories('.')
  ]
)

# Tests
if get_option('build_tests')
  gtest_dep = dependency('gtest', main : true)
  gmock_dep = dependency('gmock')
  
  test_exe = executable('snmp_client_test',
    'tests/snmp_client_test.cpp',
    dependencies : [
      gtest_dep,
      gmock_dep,
      netsnmp_dep
    ],
    include_directories : [
      include_directories('.')
    ]
  )
  
  test('snmp client tests', test_exe)
endif

add_executable(fierro-test driver.cpp)

# ELEMENTS
target_link_libraries (fierro-test elements)
target_link_libraries (fierro-test swage)
target_link_libraries (fierro-test geometry)
target_link_libraries (fierro-test slam)

# General use libraries
target_link_libraries (fierro-test Hydro)

# CCH coding
#target_link_libraries (fierro-test CCH_Hydro)

# SGH coding
#target_link_libraries (fierro-test SGH_Hydro)

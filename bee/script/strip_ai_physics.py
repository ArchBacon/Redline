#!/usr/bin/python
import strip

# list of files to remove from both the project file and from disk
files_to_remove = ["include\\ai\\navigation_system.hpp",
                   "include\\ai\\navmesh.hpp",
                   "include\\ai\\navmesh_agent.hpp",
                   "include\\graph\\euclidean_graph.hpp",
                   "include\\graph\\graph.hpp",
                   "include\\graph\\graph_search.hpp",
                   "include\\physics\\objects.hpp",
                   "include\\physics\\world.hpp",
                   "source\\ai\\navigation_system.cpp",
                   "source\\ai\\navmesh.cpp",
                   "source\\ai\\navmesh_agent.cpp",
                   "source\\graph\\euclidean_graph.cpp",
                   "source\\graph\\graph_search.cpp",
                   "source\\physics\\world.hpp",
                   "source\\model_viewer.hpp",
                   "source\\model_viewer.cpp",
                   "source\\rts.hpp",
                   "source\\rts.cpp"]

# stripping the files from the solution file
strip.strip_from_solution('../bee.sln', files_to_remove)

# removing the files from disk
strip.strip_from_disc(files_to_remove)

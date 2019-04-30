# Setting bash colors
RED    := $(shell tput -Txterm setaf 1)
YELLOW := $(shell tput -Txterm setaf 3)
GREEN  := $(shell tput -Txterm setaf 2)
PURPLE := $(shell tput -Txterm setaf 5)
WHITE  := $(shell tput -Txterm setaf 9)
LBLUE  := $(shell tput -Txterm setaf 6)
RESET  := $(shell tput -Txterm sgr0   )

#################################################################
#                         COMPILE OPTIONS                       #
#################################################################

OMP     ?= 0
MPI     ?= 0
VERBOSE ?= 1

STD    := -std=c++14
CFLAGS := -Wall -Wextra -Wno-unused-result -Wpedantic

#################################################################
#                         PARSE OPTIONS                         #
#################################################################

ifneq ($(STD), -std=c++14)
$(error $(RED)C++ minimum standard required is c++14$(RESET))
endif

omp_check := $(shell echo |cpp -fopenmp -dM | grep -i open | cut -d' ' -f 3)
ifneq ($(shell expr $(omp_check) \>= 201307), 1)
$(error $(RED)Your OpenMP is too old. Required OpenMP 4.0. Please upgrade.$(RESET))
endif

define config
	$(if $(filter $(1), $(2)), $(3), $(4) )
endef

CFLAGS   += $(strip $(call config, $(OMP),     1, -fopenmp, ))
CFLAGS   += $(strip $(call config, $(VERBOSE), 1, -DVERBOSE, ))
LDFLAGS  += $(strip $(call config, $(MPI),     1, -lboost_mpi -lboost_serialization, ))

OPTS     := $(strip $(call config, $(DEBUG),   1, -O0 -g -DDEBUG, -O3 -mavx))
MPI_OPTS := $(strip $(call config, $(MPI),     1, -D_MPI, ))

IM_OPTS  := $(strip $(call config, $(IMAGE),   1, -D__images__, ))

LDFLAGS  += -pthread

#################################################################
#                         SETTING DIRECTORIES                   #
#################################################################

SRC_DIR := ./src
INC_DIR := ./include
EXAMPLE := ./example
OBJ_DIR := ./obj
DEP_DIR := ./.dep
OUT_DIR := ./bin

SOBJDIR := ./obj/src
SDEPDIR := ./.dep/src

SRC     := $(shell find $(SRC_DIR) -name "*.cpp")
HEADER  := $(shell find $(INC_DIR) -name "*.h*")
OBJS    := $(patsubst $(SRC_DIR)/%.cpp, $(SOBJDIR)/%.o, $(SRC))
DEPS    := $(patsubst $(SRC_DIR)/%.cpp, $(SDEPDIR)/%.d, $(SRC))
EXE     := $(sort $(wildcard $(EXAMPLE)/*.cpp))

STREE   := $(sort $(patsubst %/,%,$(dir $(OBJS))))

INC     := -I$(INC_DIR)
SCPPFLAGS := -MMD -MP -MF $(@:$(SOBJDIR)/%.o=$(SDEPDIR)/%.d)

#################################################################
#                         OS FUNCTIONS                          #
#################################################################

define MKDIR
	$(if $(filter $(OS), Windows_NT), mkdir $(subst /,\,$(1)) > nul 2>&1 || (exit 0), mkdir -p $(1) )
endef

mkdir_dep  := $(call MKDIR, $(DEP_DIR))
mkdir_obj  := $(call MKDIR, $(OBJ_DIR))
mkdir_out  := $(call MKDIR, $(OUT_DIR))

#################################################################
#                         BUILD COMMAND LINE                    #
#################################################################

CFLAGS += $(STD)
CFLAGS += $(OPTS)
CFLAGS += $(INC)

all: help

#################################################################
#                         MAIN RULES                            #
#################################################################


omp: | $(OBJS) $(OUT_DIR) check-omp   				   ##@examples Compile the omp version of the genetic algorithm
		@printf "%-80s " "Compiling genetic algorithm omp version ..."
		@$(CXX) $(OBJS) $(CFLAGS) $(EXAMPLE)/omp_gen.cpp -o $(OUT_DIR)/omp_gen $(LDFLAGS)
		@printf "[done]\n"

image: CFLAGS += -D__images__
image: LDFLAGS += `pkg-config --libs opencv`

image: | $(OBJS) $(OUT_DIR) check-omp            ##@examples Compile the genetic algorithm for image reconstruction
		@printf "%-80s " "Compiling genetic algorithm image omp version ..."
		@$(CXX) $(OBJS) $(CFLAGS) $(EXAMPLE)/ga_image.cpp -o $(OUT_DIR)/ga_image $(LDFLAGS)
		@printf "[done]\n"

mpi: | $(OBJS) $(OUT_DIR) check-mpi check-omp    ##@examples Compile the mpi version of the genetic algorithm
		@printf "%-80s " "Compiling genetic algorithm mpi version ..."
		@$(OMPI_CXX) $(LDFLAGS) $(OBJS) $(CFLAGS) $(MPI_OPTS) $(EXAMPLE)/boost_mpi_gen.cpp -o $(OUT_DIR)/mpi_gen
		@printf "[done]\n"

#################################################################
#                         UTILS RULES                           #
#################################################################

.SECONDEXPANSION:
$(SOBJDIR)/%.o: $(SRC_DIR)/%.cpp | $$(@D)
		@printf "%-80s " "generating obj for $<"
		@$(CXX) $(SCPPFLAGS) $(DFLAGS) $(IM_OPTS) $(CFLAGS) -o $@ -c $<
		@printf "[done]\n"

# Add the following 'help' target to your Makefile
# And add help text after each target name starting with '\#\#'
# A category can be added with @category
HELP_FUN = \
		%help; \
		while(<>) { push @{$$help{$$2 // 'options'}}, [$$1, $$3] if /^([a-zA-Z\-]+)\s*:.*\#\#(?:@([a-zA-Z\-]+))?\s(.*)$$/ }; \
		print "\t\t\t$(LBLUE)Genetic Algorithm Makefile$(RESET)\n"; \
		print "usage: ${PURPLE}make${RESET} ${GREEN}<target>${RESET}\n\n"; \
		for (sort keys %help) { \
		print "${WHITE}$$_:${RESET}\n"; \
		for (@{$$help{$$_}}) { \
		$$sep = " " x (32 - length $$_->[0]); \
		print "  ${PURPLE}$$_->[0]${RESET}$$sep${GREEN}$$_->[1]${RESET}\n"; \
		}; \
		print "\n"; }

help:                   				     ##@utils Show this help message.
		@perl -e '$(HELP_FUN)' $(MAKEFILE_LIST)

clean:                               ##@utils Clean all files.
		@printf "%-80s " "Cleaning all files..."
		@$(RM) -r $(OBJ_DIR) $(DEP_DIR) $(OUT_DIR)
		@printf "[done]\n"

$(DEP_DIR):                          ##@utils Make dependencies directory
		@printf "%-80s " "Creating dependencies directory ..."
		@$(mkdir_dep)
		@printf "[done]\n"
$(OBJ_DIR):                          ##@utils Make objs directory.
		@printf "%-80s " "Creating objs directory ..."
		@$(mkdir_obj)
		@printf "[done]\n"
$(OBJS_DIR): %:                      ##@utils Make objs directory.
		@printf "%-80s " "Creating objs directory ..."
		@mkdir -p $@
		@printf "[done]\n"
		@printf "%-80s " "Creating dependencies directory ..."
		@mkdir -p $(@:$(SOBJDIR)%=$(SDEPDIR)%)
		@printf "[done]\n"
$(OUT_DIR):                          ##@utils Make output (executables) directory.
		@printf "%-80s " "Creating output directory ..."
		@$(mkdir_out)
		@printf "[done]\n"

$(STREE): %:												 ##@utils Make obj tree directories.
		@printf "%-80s " "Creating obj directory ..."
		@mkdir -p $@
		@printf "[done]\n"
		@printf "%-80s " "Creating dependencies directory ..."
		@mkdir -p $(@:$(SOBJDIR)%=$(SDEPDIR)%)
		@printf "[done]\n"

check-omp:													##@utils Verify omp enable.
		@if test "$(OMP)" = "0" ; then \
        echo "${YELLOW}Warning! OMP not set!${RESET}"; \
    fi;

check-mpi:													##@utils Verify mpi enable.
		@if test "$(MPI)" = "0"; then \
				echo "${RED}Error! MPI not set!${RESET}"; \
				exit 1; \
		fi;

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

OMP     := 1
MPI     := 1

STD    := -std=c++14
CFLAGS := -Wall -Wextra -Wno-unused-result

#################################################################
#                         PARSE OPTIONS                         #
#################################################################

define config
	$(if $(filter $(1), $(2)), $(3), $(4) )
endef

CFLAGS   += $(strip $(call config, $(OMP),     1, -fopenmp, ))
LDFLAGS  += $(strip $(call config, $(MPI),     1, -lboost_mpi -lboost_serialization, ))

OPTS     := $(strip $(call config, $(DEBUG),   1, -O0 -g -DDEBUG, -O3 -mavx))
MPI_OPTS := $(strip $(call config, $(MPI),     1, -D_MPI, ))

ifneq ($(STD), -std=c++14)
$(error $(RED)C++ minimum standard required is c++14$(RESET))
endif

#################################################################
#                         SETTING DIRECTORIES                   #
#################################################################

SRC_DIR    := ./src
INC_DIR    := ./include
OUT_DIR    := ./bin

INC        := -I$(INC_DIR)

#################################################################
#                         OS FUNCTIONS                          #
#################################################################

define MKDIR
	$(if $(filter $(OS), Windows_NT), mkdir $(subst /,\,$(1)) > nul 2>&1 || (exit 0), mkdir -p $(1) )
endef

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


omp: | $(OUT_DIR) check-omp   				 ##@examples Compile the omp version of the genetic algorithm
		@printf "%-80s " "Compiling genetic algorithm omp version ..."
		@$(CXX) $(CFLAGS) $(SRC_DIR)/omp_gen.cpp -o $(OUT_DIR)/omp_gen
		@printf "[done]\n"

mpi: | $(OUT_DIR) check-mpi check-omp  ##@examples Compile the mpi version of the genetic algorithm
		@printf "%-80s " "Compiling genetic algorithm mpi version ..."
		@$(OMPI_CXX) $(LDFLAGS) $(CFLAGS) $(MPI_OPTS) $(SRC_DIR)/boost_mpi_gen.cpp -o $(OUT_DIR)/mpi_gen
		@printf "[done]\n"

#################################################################
#                         UTILS RULES                           #
#################################################################

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

help:                   				##@utils Show this help message.
		@perl -e '$(HELP_FUN)' $(MAKEFILE_LIST)

$(OUT_DIR):             				##@utils Make output (executables) directory.
		@printf "%-80s " "Creating output directory ..."
		@$(mkdir_out)
		@printf "[done]\n"


check-omp:
		@if test "$(OMP)" = "0" ; then \
        echo "${YELLOW}Warning! OMP not set!${RESET}"; \
    fi;

check-mpi:
		@if test "$(MPI)" = "0"; then \
				echo "${RED}Error! MPI not set!${RESET}"; \
				exit 1; \
		fi;
MODULES = stablo
PGXS := $(shell pg_config --pgxs)
include $(PGXS)
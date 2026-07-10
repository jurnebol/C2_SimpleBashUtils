.PHONY: all s21_cat s21_grep test style clean rebuild

all: s21_cat s21_grep

s21_cat:
	$(MAKE) -C ./cat

s21_grep:
	$(MAKE) -C ./grep

test: all
	bash ./tests/run_tests.sh

style:
	clang-format -i ./cat/*.c ./cat/*.h ./common/*.c ./common/*.h ./grep/*.c ./grep/*.h

clean:
	$(MAKE) -C ./cat clean
	$(MAKE) -C ./grep clean

rebuild: clean all

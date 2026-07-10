#!/usr/bin/env bash

set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
S21_CAT="$ROOT/cat/s21_cat"
S21_GREP="$ROOT/grep/s21_grep"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

PASS=0
FAIL=0

printf 'hello\tworld\n\n\nfoo\nbar\n\n' > "$WORK/t1.txt"
printf 'line1\nline2\n' > "$WORK/t2.txt"
printf 'Hello World\nfoo bar\nFOOBAR\nbaz\nhello again\n\nqux foo\n' > "$WORK/g1.txt"
printf 'second file foo\nnothing here\n' > "$WORK/g2.txt"
printf 'foo\nbaz\n' > "$WORK/patterns.txt"

have_busybox=0
command -v busybox >/dev/null 2>&1 && have_busybox=1

check_cat() {
  local ref="$1"; shift
  local a b
  a=$($ref "$@" 2>/dev/null)
  b=$($S21_CAT "$@" 2>/dev/null)
  if [ "$a" = "$b" ]; then
    PASS=$((PASS + 1))
  else
    FAIL=$((FAIL + 1))
    echo "[cat FAIL] ref=$ref args: $*"
    diff <(echo "$a") <(echo "$b") | sed 's/^/    /'
  fi
}

check_grep() {
  local ref="$1"; shift
  local a b
  a=$($ref "$@" 2>/dev/null; echo "EXIT:$?")
  b=$($S21_GREP "$@" 2>/dev/null; echo "EXIT:$?")
  if [ "$a" = "$b" ]; then
    PASS=$((PASS + 1))
  else
    FAIL=$((FAIL + 1))
    echo "[grep FAIL] ref=$ref args: $*"
    diff <(echo "$a") <(echo "$b") | sed 's/^/    /'
  fi
}

for flags in "" "-b" "-n" "-s" "-e" "-t" "-E" "-T" "-v" \
             "-bs" "-ns" "-nE" "-bE" "-es"; do
  check_cat cat $flags "$WORK/t1.txt" "$WORK/t2.txt"
done
check_cat cat "$WORK/t1.txt"

if [ "$have_busybox" = 1 ]; then
  for flags in "" "-b" "-n" "-e" "-t" "-v" "-be" "-ne" "-nt" "-nv" "-bt"; do
    check_cat "busybox cat" $flags "$WORK/t1.txt" "$WORK/t2.txt"
  done
fi

for args in "foo $WORK/g1.txt" \
            "foo $WORK/g1.txt $WORK/g2.txt" \
            "-i foo $WORK/g1.txt" \
            "-v foo $WORK/g1.txt" \
            "-c foo $WORK/g1.txt $WORK/g2.txt" \
            "-l foo $WORK/g1.txt $WORK/g2.txt" \
            "-n foo $WORK/g1.txt" \
            "-h foo $WORK/g1.txt $WORK/g2.txt" \
            "-o foo $WORK/g1.txt" \
            "-e foo -e baz $WORK/g1.txt" \
            "-f $WORK/patterns.txt $WORK/g1.txt" \
            "-s foo /no/such/file" \
            "-in foo $WORK/g1.txt" \
            "-iv foo $WORK/g1.txt" \
            "-ic foo $WORK/g1.txt $WORK/g2.txt" \
            "-on foo $WORK/g1.txt" \
            "-vc foo $WORK/g1.txt"; do
  check_grep grep $args
done

echo "----------------------------------------"
echo "PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ]

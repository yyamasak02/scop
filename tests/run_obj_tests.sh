#!/usr/bin/env bash
# OBJパーサーのデータ駆動テスト。ケース定義は tests/cases.json、入力は tests/cases/*.obj。
# 必要なもの: c++(ASan対応)、python3(JSON読み込み用)
#   ./tests/run_obj_tests.sh                 全ケース実行
#   ./tests/run_obj_tests.sh -v              失敗時/成功時のメッセージも表示
#   ./tests/run_obj_tests.sh <カテゴリ|部分一致>   例: 凹多角形 / err_face
#
# ASan/UBSan と libstdc++ の境界チェック付きでビルドするので、
# 範囲外アクセスや未定義動作もクラッシュとして検出される。
set -u
cd "$(dirname "$0")/.." || exit 1

VERBOSE=0
FILTER=""
for a in "$@"; do
  if [ "$a" = "-v" ]; then VERBOSE=1; else FILTER=$a; fi
done

BIN=tests/.obj_harness
CXX=${CXX:-c++}
$CXX -std=c++20 -g -O0 -fsanitize=address,undefined -fno-sanitize-recover=undefined \
  -D_GLIBCXX_ASSERTIONS -Iincludes \
  tests/obj_harness.cpp srcs/WaveFrontObjectLoader.cpp srcs/Vec.cpp \
  -o "$BIN" || { echo "BUILD FAILED"; exit 1; }

# 負荷テスト用の大きなOBJはリポジトリに入れず、毎回生成する(3MB弱)
[ -f tests/cases/ok_large.obj ] || awk 'BEGIN{for(i=0;i<100000;i++)printf "v %d %d %d\n",i%100,(i/100)%100,i%7; for(i=1;i<=99998;i++)printf "f %d %d %d\n",i,i+1,i+2}' > tests/cases/ok_large.obj

# 大きな面の性能テスト
[ -f tests/cases/ok_ngon_large.obj ] || awk 'BEGIN{n=20000; print "# EXPECT_TRIS " n-2; for(i=0;i<n;i++){a=6.283185307179586*i/n; printf "v %.9f %.9f 0\n",cos(a),sin(a)}; printf "f"; for(i=1;i<=n;i++) printf " %d",i; print ""}' > tests/cases/ok_ngon_large.obj

pass=0; fail=0; failed=()
declare -A listed

printf '%-6s %-4s %-8s %-38s %s\n' RESULT 期待 カテゴリ ファイル 確かめること
printf -- '------------------------------------------------------------------------------------------------\n'

while IFS=$'\x1f' read -r file expect cat desc; do
  listed[$file]=1

  case $file in
    @nonexistent) path="tests/cases/__no_such_file__.obj" ;;
    @directory)   path="tests/cases" ;;
    @empty)       path="" ;;
    *)            path="tests/cases/$file" ;;
  esac

  if [ -n "$FILTER" ] && [[ "$file $cat $desc" != *"$FILTER"* ]]; then continue; fi

  if [ -n "$path" ] && [ "${file:0:1}" != "@" ] && [ ! -f "$path" ]; then
    out="ケースファイルがない"; rc=99
  else
    out=$(timeout 10 "$BIN" "$path" 2>&1); rc=$?
  fi

  verdict=FAIL; reason=""
  if [ $rc -eq 99 ]; then reason=$out
  elif [ $rc -eq 124 ]; then reason="TIMEOUT(ハング)"
  elif [ $rc -gt 2 ]; then reason="CRASH rc=$rc"
  elif [ "$expect" = ok ]; then
    if [ $rc -eq 0 ]; then verdict=PASS; else reason="読めるべきなのに: ${out%%$'\n'*}"; fi
  else
    if [ $rc -eq 1 ]; then verdict=PASS; else reason="拒否すべきなのに: ${out%%$'\n'*}"; fi
  fi

  if [ $verdict = PASS ]; then
    pass=$((pass+1)); mark=$'\033[32mPASS\033[0m  '
  else
    fail=$((fail+1)); failed+=("$file"); mark=$'\033[31mFAIL\033[0m  '
  fi
  printf '%s %-4s %-8s %-38s %s\n' "$mark" "$expect" "$cat" "$file" "$desc"
  if [ $verdict = FAIL ]; then printf '         └─ %s\n' "$reason"
  elif [ $VERBOSE -eq 1 ]; then printf '         └─ %s\n' "${out%%$'\n'*}"; fi
done < <(python3 -I -c '
import json, sys
for c in json.load(open("tests/cases.json", encoding="utf-8"))["cases"]:
    print("\x1f".join([c["file"], c["expect"], c["category"], c["desc"]]))
')

# 表にない .obj を検出(足し忘れ防止)
if [ -z "$FILTER" ]; then
  for f in tests/cases/*.obj; do
    b=$(basename "$f")
    if [ -z "${listed[$b]:-}" ]; then
      fail=$((fail+1)); failed+=("$b"); printf '\033[31mFAIL\033[0m   cases.txt に未登録: %s\n' "$b"
    fi
  done
fi

echo
echo "RESULT: $pass passed, $fail failed"
if [ $fail -gt 0 ]; then
  printf 'failed: %s\n' "${failed[*]}"
  exit 1
fi

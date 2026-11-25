// Copyright [2026] yyamasak
#pragma once
#include <source_location>
#include <string>

// アプリの出力はすべてここを通す(std::cout / std::cerr を直接使わない)。
//  info  : 通常の案内。標準出力へ
//  warn  : 警告。標準エラーへ
//  error : エラー。標準エラーへ
// エラーを標準エラーに分けるのは、`./scop > log.txt` でも画面に出るようにするため。
//
// 出力形式:
//   2026-11-25 21:50:12.345 [ERROR] [Shader::checkCompileErrors] message
//   └ 時刻(ミリ秒)          └ レベル └ 呼び出し元のクラス::メソッド
// 呼び出し元は std::source_location の既定引数で自動取得する
// (呼び出し側は Logger::error("...") と書くだけでよい)。
class Logger {
 public:
  static void info(const std::string& message,
                   std::source_location where = std::source_location::current());
  static void warn(const std::string& message,
                   std::source_location where = std::source_location::current());
  static void error(const std::string& message,
                    std::source_location where = std::source_location::current());

 private:
  Logger() = delete;  // 静的メソッドだけのクラス
};

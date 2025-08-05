# Equivalent to: clang-format --dry-run --Werror `find ./ -type f -name "*.h" -o -name "*.cc" -o -name "*.cpp"`
Get-ChildItem -Recurse -Include *.h,*.cc,*.cpp -Path ./include,./src | ForEach-Object {
    clang-format --dry-run $_.FullName
}
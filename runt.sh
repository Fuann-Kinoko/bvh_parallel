#!/bin/bash
cmake --build build

cp build/BVHVisualization ./BVHVisualization
# 定义文件参数
INPUT_CSV="oncetime/oncetime.csv"    # 缓存表格
OUTPUT_CSV="runtime.csv"    # 汇总表格

DEFAULT_MODEL="Cow"
MODEL="$DEFAULT_MODEL"
# 运行程序
./BVHVisualization --no_gui "$MODEL"

if [ $# -ge 1 ]; then
    case "$1" in
        Dragon|Cow|Face|Car)
            MODEL="$1"
            ;;
        *)
            echo "error：invalid arg '$1'"
            echo "avaliable: Dragon, Cow, Face, Car"
            exit 1
            ;;
    esac
fi

# 计算次数
if [ -f "$OUTPUT_CSV" ]; then
    # 获取当前最大运行次数
    LAST_RUN=$(awk -F, 'BEGIN{max=0} {if($1>max)max=$1} END{print max}' "$OUTPUT_CSV")
    RUN_NUMBER=$((LAST_RUN + 1))
else
    RUN_NUMBER=1
fi

if [ -f "$INPUT_CSV" ]; then
    if [ ! -s "$INPUT_CSV" ]; then
        echo "错误：运行时数据文件 $INPUT_CSV 为空"
        exit 1
    fi

    # 格式：运行次数,模型,时间
    awk -v run="$RUN_NUMBER" -v model="$MODEL" '
        BEGIN { OFS="," }
        { print run, model, $0 }
    ' "$INPUT_CSV" >> "$OUTPUT_CSV"

    # 生成统计信息
    TOTAL_RENDERS=$(wc -l < "$INPUT_CSV")
    AVERAGE_TIME=$(awk '{sum+=$1} END{printf "%.4f", sum/NR}' "$INPUT_CSV")

    echo "==== 运行完成！ ===="
    echo "运行次数:  第${RUN_NUMBER}次"
    echo "模型类型:      $MODEL"

    # 清空原始数据文件
    > "$INPUT_CSV"
else
    echo "错误：未找到运行时数据文件 $INPUT_CSV"
    exit 1
fi
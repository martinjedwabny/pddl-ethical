#!/usr/bin/env bash

usage(){
  echo "Usage:
    $0 -c|--command \"string\" -d|--datasets file1 [file2 ...]
      -q|--querysets file1 [file2 ...] [-r|--ram 126m [512m ...]] [-i|--iterations 5] [-d|--directory benchmark/]" 1>&2
  echo "Example:
    $0 -c \"grafthug.jar -i __DATASET_ -q __QUERYSET__\" -d datasets/*
     -q querysets/* -r 2g 4g" 1>&2
  echo "Note:
    * -r,--ram must be conform to the -Xmx java command
    * -i,--iterations defines how many iterations are done, <5> by default
    * -d,--directory defines where the logs will be saved, <benchmark> by default
    * the command string will automatically replace __QUERYSET__ and
      __DATASET__ by the appropriate file when parsing
    * the standart output of the files will be saved in
        <benchmark>/<ram>_<dataset>_<queryset>.txt" 1>&2
  exit 1
}

DOING="\e[93m[DOING]\e[0m"
DONE="\e[32m[ DONE]\e[0m"

each_bench(){
    #echo "DATA: $1 QUERY: $2 RAM: $3"
    bench_cmd="${cmd/__DATASET__/$1}"
    bench_cmd="${bench_cmd/__QUERYSET__/$2}"

    display_cmd="${cmd/__DATASET__/'\e[94m'$1'\e[0m'}"
    display_cmd="${display_cmd/__QUERYSET__/'\e[94m'$2'\e[0m'}"

    data_filename=$(basename "$1")
    query_filename=$(basename "$2")
    data_filename_no_ext="${data_filename%%.*}"
    query_filename_no_ext="${query_filename%%.*}"


    # initializing the file
    echo -n > "${bench_dir}/${3}_${data_filename_no_ext}_${query_filename_no_ext}.txt"

    printf "\r$DOING[%${total_width}d/%d][%${iter_width}d/%d][RAM: \e[94m%b\e[0m ] %b" ${current_count} ${total_count} 0 ${iterations} "${3}" "${display_cmd}"


    for (( i = 0; i < $iterations; i++ )); do
        #echo -ne "$((i+1))/$iterations\r"

        echo -e "### Iteration : $((i+1))/${iterations}" >> "${bench_dir}/${3}_${data_filename_no_ext}_${query_filename_no_ext}.txt"
        if [[ $3 == "" ]]; then # change here
            { time -p java -jar $bench_cmd; } >> "${bench_dir}/${3}_${data_filename_no_ext}_${query_filename_no_ext}.txt" 2>&1
        else # or here, i don't know how it works anymore :(
            { time -p java -Xmx${3} -jar $bench_cmd; } >> "${bench_dir}/${3}_${data_filename_no_ext}_${query_filename_no_ext}.txt" 2>&1
        fi

        printf "\r$DOING[%${total_width}d/%d][%${iter_width}d/%d]" ${current_count} ${total_count} $((i+1)) ${iterations}

    done
    echo -e "\r$DONE"
    current_count=$((current_count+1))
}

iterations=5
cmd=""
total_count=0
current_count=1
bench_dir="benchmark"
lastcommand=""

for arg in "$@"; do
    case "$arg" in
        -h | --help)
            usage
            ;;
        -c | --command)
            lastcommand="command"
            ;;
        -i | --iterations)
            lastcommand="iterations"
            ;;
        -d | --datasets)
            lastcommand="datasets"
            ;;
        -q | --querysets)
            lastcommand="querysets"
            ;;
        -r | --ram)
            lastcommand="ram"
            ;;
        -d | --directory)
            lastcommand="dir"
            ;;
        *)
            if [[ $lastcommand == "command" ]]; then
                cmd="$arg"
                # echo $arg
            elif [[ $lastcommand == "iterations" ]]; then
                iterations="$arg"
                # echo $arg
            elif [[ $lastcommand == "dir" ]]; then
                bench_dir="$arg"
                # echo $arg
            elif [[ $lastcommand == "querysets" ]]; then
                querysets+=("$arg")
            elif [[ $lastcommand == "datasets" ]]; then
                datasets+=("$arg")
            elif [[ $lastcommand == "ram" ]]; then
                ramset+=("$arg")
            fi
            ;;
    esac
done

if [[ -z  ${ramset+x} ]]; then
    #if ramset is not set
    # echo "$cmd Q:${querysets[*]} D:${datasets[*]}"
    ramset=("")
fi

ram_count=${#ramset[@]}
data_count=${#datasets[@]}
query_count=${#querysets[@]}
total_count=$((ram_count*data_count*query_count))
total_width=${#total_count}
iter_width=${#iterations}

if [[ $total_count == 0 ]]; then # either data or query are not set
    usage
fi

# creating benchmark directory
mkdir -p ${bench_dir}

for ram in "${ramset[@]}"; do
    for data in "${datasets[@]}"; do
        for query in "${querysets[@]}"; do
            each_bench "$data" "$query" "$ram"
        done
    done
done

echo "DONE"

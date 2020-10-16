bin="$(pwd)/build/bin/fdr"
datasetroot="$(pwd)/data"
resultsroot="$(pwd)/results/nodar/disp_0"

echo ${bin}

mkdir -p ${resultsroot}
${bin} -targetDir ${datasetroot}/nodar -outputDir ${resultsroot} -mode NODAR -lambda 0.30 -seg_k 80.0 -inlier_ratio 0.50 -ndisp 256 -use_swap false

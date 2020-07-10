pass_str[0]="TEST PASSED"
pass_str[1]="TEST NOT PASSED"
:>log.txt
for fn in test_fast_iaf_psc_exp.py test_fixed_total_number.py test_iaf_psc_exp.py test_spike_times.py test_aeif_cond_alpha.py  test_aeif_cond_beta.py  test_aeif_psc_alpha.py  test_aeif_psc_delta.py  test_aeif_psc_exp.py test_stdp_list.py test_stdp.py test_syn_model.py test_brunel_list.py test_brunel_outdegree.py test_brunel_user_m1.py test_spike_detector.py; do
    python $fn >> log.txt 2>err.txt
    res=$?
    cat err.txt >> log.txt
    rm -f err.txt
    if [ "$res" -ne "0" ]; then
	res=1
    fi
    echo ${pass_str[$res]}
done
for fn in syn_group connect getarr setvar2; do
    python test_$fn.py 2>&1 | grep -v dyl > tmp
    diff -qs tmp log_$fn.txt 2>&1 >> log.txt
    res=$?
    echo ${pass_str[$res]}    
done
rm -f tmp

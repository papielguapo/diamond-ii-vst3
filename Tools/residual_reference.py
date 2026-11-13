"""Measured Dark Essence/Gold residual model, 48 kHz only.

Fit candidates from user-owned PCM24 captures or render the published curve.
No automatic source/header replacement is performed by fitting.
"""
import argparse,json
from pathlib import Path
import numpy as np
from scipy.interpolate import CubicSpline
from scipy.optimize import least_squares
from scipy.signal import lfilter
from reference_math import read,shelf,model as legacy_model

PROJECT=Path(__file__).resolve().parents[1]

def parameters(config):
    return np.r_[config['residual_values'][1:-1],[config[k] for k in ('preFrequency','preDb','postFrequency','postDb','preDamping','postDamping','phaseMemory')]]

def render(x,config):
    b,a=shelf(config['preFrequency'],config['preDb'],config['preDamping']); u=lfilter(b,a,x)
    curve=CubicSpline(config['knots'],config['residual_values'],bc_type=((2,0),(1,0)))
    residual=np.where(np.abs(u)<1,np.sign(u)*curve(np.minimum(np.abs(u),1)),0.)
    previous=np.r_[0.,residual[:-1]]
    corrected=residual+config['phaseMemory']*u*u/(1+u*u)*(residual-previous)
    b,a=shelf(config['postFrequency'],config['postDb'],config['postDamping'])
    return x+lfilter(b,a,corrected)

def render_process(x,config,amount,correction):
    old=legacy_model(x,config['low_amount_legacy_parameters'])-x
    new=render(x,config)-x
    blend=np.clip((amount-.75)*4,0,1); wet=old+blend*(new-old)
    return x+amount*wet+4*amount*(1-amount)*(correction[0]*x+correction[1]*wet)

def config_from_parameters(p,initial):
    result=dict(initial);n=len(initial['knots'])-2
    result['residual_values']=[0.]+p[:n].tolist()+[0.]
    for key,value in zip(('preFrequency','preDb','postFrequency','postDb','preDamping','postDamping','phaseMemory'),p[n:]): result[key]=float(value)
    return result

def fit(reference_dir,initial):
    source=read(reference_dir/'01_probe_48k.wav');segments=json.loads((reference_dir/'segments.json').read_text())['segments']
    inputs=[];expected=[];weights=[]
    for trim in (0,6):
        target=read(reference_dir/f"PhoenixII_48k_{initial['type']}_{initial['color']}_P100_In{trim}_Outm6.wav")[96000:96000+len(source)]*10**(.3)
        prior=legacy_model(source*10**(trim/20),initial['low_amount_legacy_parameters'])
        for s in segments:
            if not ((s['kind']=='sine' and s['frequency_hz'] in (1000,250,8000)) or s['name']=='two_tone_19000_20000Hz'): continue
            start=s['start_sample'];inputs.append(source[start:start+14400]*10**(trim/20))
            y=target[start+12000:start+14400]; baseline=prior[start+12000:start+14400];rms=np.sqrt(np.mean(y*y));relative=np.sqrt(np.mean((y-baseline)**2))/rms
            expected.append(y);weights.append(np.full(2400,1/(rms*max(relative,.005)**.5)))
    y=np.concatenate(expected);w=np.concatenate(weights);n=len(initial['knots'])-2
    def residual(p):
        cfg=config_from_parameters(p,initial)
        return (np.concatenate([render(x,cfg)[12000:] for x in inputs])-y)*w
    result=least_squares(residual,parameters(initial),bounds=(np.r_[np.full(n,-2.),100,-12,100,-24,.3,.3,-4],np.r_[np.full(n,2.),18000,18,18000,12,5,5,8]),max_nfev=400,diff_step=1e-4)
    return config_from_parameters(result.x,initial)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('reference_dir',type=Path)
    parser.add_argument('--initial',type=Path,default=PROJECT/'Calibration/residual-dark-gold.json')
    parser.add_argument('--output',type=Path,default=Path('residual-candidate.json'))
    args=parser.parse_args();cfg=fit(args.reference_dir,json.loads(args.initial.read_text()));args.output.write_text(json.dumps(cfg,indent=2))
    print('Candidate written to',args.output,'; compare untrained captures before adoption.')

if __name__=='__main__': main()

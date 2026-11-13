import json,struct,argparse
from pathlib import Path
import numpy as np
from scipy.signal import lfilter
from scipy.optimize import least_squares
def read(path):
    b=Path(path).read_bytes(); i=12
    while i+8<=len(b):
        k,n=struct.unpack_from('<4sI',b,i)
        if k==b'data':
            u=np.frombuffer(b[i+8:i+8+n],np.uint8).reshape(-1,3).astype(np.int32)
            return ((u[:,0]|u[:,1]<<8|u[:,2]<<16)^0x800000).astype(np.float64)/8388608-1.
        i+=8+n+(n&1)
    raise ValueError('No audio')
def shelf(fc,db):
    k=np.tan(np.pi*fc/48000); v=10**(db/20); n=1/(1+np.sqrt(2)*k+k*k)
    return np.array([v+np.sqrt(2*v)*k+k*k,2*(k*k-v),v-np.sqrt(2*v)*k+k*k])*n,np.array([1,2*(k*k-1)*n,(1-np.sqrt(2)*k+k*k)*n])
def model(x,p,family):
    g,k,q,fc,db,postfc,postdb=p; b,a=shelf(fc,db); u=lfilter(b,a,x)
    if family=='general': y=g*u/(1+np.abs(k*u)**q)**(1/q)
    elif family=='tanh': y=g*np.tanh(k*u)/k
    elif family=='atan': y=g*np.arctan(k*u)/k
    postb,posta=shelf(postfc,postdb)
    return lfilter(postb,posta,y)

parser=argparse.ArgumentParser(description="Fit user-owned PCM24 48 kHz reference captures; see Calibration/README.md")
parser.add_argument("reference_dir",type=Path)
parser.add_argument("--initial",type=Path,default=Path(__file__).resolve().parents[1]/"Calibration/gold-48k-fit.json")
parser.add_argument("--output",type=Path,default=Path("fitted-models.json"))
parser.add_argument("--colors",nargs="+",default=["Opal","Gold","Sapphire"])
args=parser.parse_args(); REF=args.reference_dir
ss=json.loads((REF/'segments.json').read_text())['segments']; source=read(REF/'01_probe_48k.wav'); gold=json.loads(args.initial.read_text()); out=args.output; results=json.loads(out.read_text()) if out.exists() else []
for color in args.colors:
 for idx,entry in enumerate(gold):
  name=entry['type']; path=REF/f'PhoenixII_48k_{name}_{color}_P100_In0_Outm6.wav'
  if not path.exists() or any(r['color']==color and r['type']==name for r in results):continue
  target=read(path)[96000:96000+len(source)]*10**(.3); train=[s for s in ss if (s['kind']=='sine' and s['frequency_hz'] in (1000,250,8000)) or s['name']=='two_tone_19000_20000Hz']; xx=[]; yy=[];ww=[]
  for s in train:
   start=s['start_sample']; xx.append(source[start:start+14400]);yy.append(target[start+12000:start+14400]);ww.append(np.full(2400,10**(-s['peak_dbfs']/20) if s['kind']=='sine' else 5))
  y=np.concatenate(yy);w=np.concatenate(ww)
  def residual(p):return (np.concatenate([model(x,p,'general')[12000:] for x in xx])-y)*w
  fit=least_squares(residual,entry['parameters'],bounds=([.5,.05,1.8,100,0,100,-24],[4,8,12,18000,18,18000,0]),diff_step=1e-4,max_nfev=350)
  pred=model(source,fit.x,'general'); metrics=[]
  for s in ss:
   a=s['analysis_start_sample'] if s['kind']=='sine' else s['start_sample'];b=a+(30720 if s['kind']=='sine' else s['length_samples']);metrics.append(dict(probe=s['name'],training=s in train,error_db=float(20*np.log10(np.linalg.norm(pred[a:b]-target[a:b])/np.linalg.norm(target[a:b])))))
  results.append(dict(type=name,color=color,parameters=fit.x.tolist(),metrics=metrics));out.write_text(json.dumps(results,indent=2));print(name,color,'fit',fit.cost,'withheld',[(m['probe'],round(m['error_db'],2)) for m in metrics if not m['training']],flush=True)

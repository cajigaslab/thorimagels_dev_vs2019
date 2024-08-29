import os
import pathlib
import subprocess

def main():
  print('!', pathlib.Path.cwd())
  grpc_root = os.environ.get('GRPC_ROOT', None)
  assert grpc_root, 'GRPC_ROOT is undefined'
  grpc_root = pathlib.Path(grpc_root)

  inputs = [
    pathlib.Path('thalamus.proto'),
    pathlib.Path('util.proto'),
  ]
  outputs = [
    pathlib.Path('thalamus.pb.h'),
    pathlib.Path('thalamus.pb.cc'),
    pathlib.Path('thalamus.grpc.pb.h'),
    pathlib.Path('thalamus.grpc.pb.cc'),
    pathlib.Path('util.pb.h'),
    pathlib.Path('util.pb.cc'),
    pathlib.Path('util.grpc.pb.h'),
    pathlib.Path('util.grpc.pb.cc'),
  ]
  max_input_time = [i.stat().st_mtime for i in inputs if i.exists()]
  min_output_time = [o.stat().st_mtime for o in outputs if o.exists()]
  if min_output_time and min_output_time[0] >= max_input_time[0]:
    return
  for i in inputs:
    subprocess.check_call([grpc_root/'bin'/'protoc.exe', '--grpc_out', '.', '--cpp_out', '.', '-I', '.',
                           f'--plugin=protoc-gen-grpc={grpc_root/"bin"/"grpc_cpp_plugin.exe"}', i])

if __name__ == '__main__':
  main()

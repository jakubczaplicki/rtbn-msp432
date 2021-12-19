## Author: Ramesh Yerraballi
## Use this in octave to convert a wav file into a C declaration of the
## the samples with a chosen precision at 11.025kHz
## Run in octave as:
## WavConv ('filename', precision); 
## The inputs are (i) the name of the wav file without the extention .wav.
##                (ii) the sample precision in bits (e.g., 4 if using a 4-bit DAC)
## This will generate a filename.txt file that has the C declaration.
## For example, say you have a sound file called boom.wav that you want to use in
## in your game. 
##    1. Put the WavConv.m script and the boom.wav file in a folder (say sounds).
##    2. Open octave/matlab and cd to the sounds folder
##    3. Run script as: WavConv("boom", 4)
##       For 4-bit precision
##       This will produce a file named boom.txt, which contains a C declaration
##       for the array of samples. You can cut-and-paste it in your code
function WavConv(filename,precision)
  # 1. Read the file and extract the sample as well as the frequency:
  [Spls, fs] = wavread(strcat(filename,'.wav'));
  # 2. Downsample it, to get it to the frequency you want (11.025 kHz):
  Spls = downsample(Spls, round(fs/11025));
  # 3. Adjust the samples (originals are fractions between -1 and 1) to range between 0 and 15 (4-bit)
  #    If you want to change it to 6-bit uncomment the 6-bit 
  Spls = round((Spls+ 1)* (2^precision-1)/2); 
  # 4. write it to a file so you can cut and paste it into your C code:
  file = fopen(strcat(filename,'.txt'), 'w');
  fprintf(file, cstrcat('unsigned char ', filename,'[',num2str(length(Spls)),'] = {'));
  fprintf(file, '%d,', Spls(1:length(Spls)-1));
  fprintf(file, '%d', Spls(length(Spls)));
  fprintf(file, '};\n');
  fclose(file);
end

## Copyright (C) 2005, 2006, 2007, 2008, 2009 Michael Zeising
##
## This file is part of Octave.
function [y, samples_per_sec, bits_per_sample] = wavread (filename, param)

  FORMAT_PCM        = 0x0001;   # PCM (8/16/32 bit)
  FORMAT_IEEE_FLOAT = 0x0003;   # IEEE float (32/64 bit)
  BYTEORDER         = "ieee-le";

  if (nargin < 1 || nargin > 2)
    print_usage ();
  endif

  if (! ischar (filename))
    error ("wavwrite: expecting filename to be a character string");
  endif

  # Open file for binary reading.
  [fid, msg] = fopen (filename, "rb");
  if (fid < 0)
    error ("wavread: %s", msg);
  endif

  ## Get file size.
  fseek (fid, 0, "eof");
  file_size = ftell (fid);
  fseek (fid, 0, "bof");

  ## Find RIFF chunk.
  riff_size = find_chunk (fid, "RIFF", file_size);
  riff_pos = ftell (fid);
  if (riff_size == -1)
    fclose (fid);
    error ("wavread: file contains no RIFF chunk");
  endif

  riff_type = char (fread (fid, 4))';
  if(! strcmp (riff_type, "WAVE"))
    fclose (fid);
    error ("wavread: file contains no WAVE signature");
  endif
  riff_pos = riff_pos + 4;
  riff_size = riff_size - 4; 

  ## Find format chunk inside the RIFF chunk.
  fseek (fid, riff_pos, "bof");
  fmt_size = find_chunk (fid, "fmt ", riff_size);
  fmt_pos = ftell(fid);
  if (fmt_size == -1)
    fclose (fid);
    error ("wavread: file contains no format chunk");
  endif
 
  ## Find data chunk inside the RIFF chunk.
  ## We don't assume that it comes after the format chunk.
  fseek (fid, riff_pos, "bof");
  data_size = find_chunk (fid, "data", riff_size);
  data_pos = ftell (fid);
  if (data_size == -1)
    fclose (fid);
    error ("wavread: file contains no data chunk");
  endif
  
  ### Read format chunk.
  fseek (fid, fmt_pos, "bof");
 
  ## Sample format code.
  format_tag = fread (fid, 1, "uint16", 0, BYTEORDER);
  if (format_tag != FORMAT_PCM && format_tag != FORMAT_IEEE_FLOAT)
    fclose (fid);
    error ("wavread: sample format %#x is not supported", format_tag);
  endif

  ## Number of interleaved channels.
  channels = fread (fid, 1, "uint16", 0, BYTEORDER);

  ## Sample rate.
  samples_per_sec = fread (fid, 1, "uint32", 0, BYTEORDER);

  ## Bits per sample.
  fseek (fid, 6, "cof");
  bits_per_sample = fread (fid, 1, "uint16", 0, BYTEORDER);

  ### Read data chunk.
  fseek (fid, data_pos, "bof");
  
  ## Determine sample data type.
  if (format_tag == FORMAT_PCM)
    switch (bits_per_sample)
      case 8
        format = "uint8";
      case 16 
        format = "int16";
      case 24
	format = "uint8";
      case 32 
        format = "int32";
      otherwise
        fclose (fid);
        error ("wavread: %d bits sample resolution is not supported with PCM",
	       bits_per_sample);
    endswitch
  else
    switch (bits_per_sample)
      case 32 
        format = "float32";
      case 64 
        format = "float64";
      otherwise
        fclose (fid);
        error ("wavread: %d bits sample resolution is not supported with IEEE float",
	       bits_per_sample);
    endswitch
  endif
  
  ## Parse arguments.
  if (nargin == 1)
    length = 8 * data_size / bits_per_sample;
  else
    if (size (param, 2) == 1)
      ## Number of samples is given.
      length = param * channels;
    elseif (size (param, 2) == 2)
      ## Sample range is given.
      if (fseek (fid, (param(1)-1) * channels * (bits_per_sample/8), "cof") < 0)
        warning ("wavread: seeking failed");
      endif
      length = (param(2)-param(1)+1) * channels;
    elseif (size (param, 2) == 4 && char (param) == "size")
      ## Size of the file is requested.
      fclose (fid);
      y = [ck_size/channels/(bits_per_sample/8), channels];
      return
    else
      fclose (fid);
      error ("wavread: invalid argument 2");
    endif
  endif

  ## Read samples and close file.
  if (bits_per_sample == 24)
    length *= 3;
  endif
  [yi, n] = fread (fid, length, format, 0, BYTEORDER);
  fclose (fid);

  ## Check data.
  if (mod (numel (yi), channels) != 0)
    error ("wavread: data in %s doesn't match the number of channels",
	   filename);
  endif

  if (bits_per_sample == 24)
    yi = reshape (yi, 3, rows(yi)/3)';
    yi(yi(:,3) >= 128, 3) -= 256;
    yi = yi * [1; 256; 65536];
  endif

  if (format_tag == FORMAT_PCM)
    ## Normalize samples.
    switch (bits_per_sample)
      case 8
        yi = (yi - 128)/127;
      case 16
        yi /= 32767;
      case 24
		yi /= 8388607;
      case 32
        yi /= 2147483647;
    endswitch
  endif
  
  ## Deinterleave.
  nr = numel (yi) / channels;
  y = reshape (yi, channels, nr)';
  
endfunction

## Given a chunk_id, scan through chunks from the current file position
## though at most size bytes.  Return the size of the found chunk, with
## file position pointing to the start of the chunk data.  Return -1 for
## size if chunk is not found.

function chunk_size = find_chunk (fid, chunk_id, size)
  id = "";
  offset = 8;
  chunk_size = 0;

  while (! strcmp (id, chunk_id) && (offset < size))
    fseek (fid, chunk_size, "cof");
    id = char (fread (fid, 4))';
    chunk_size = fread (fid, 1, "uint32", 0, "ieee-le");
    offset = offset + 8 + chunk_size;
  endwhile
  if (! strcmp (id, chunk_id))
    chunk_size = -1;
  endif
endfunction

## Author: Paul Kienzle
function y = downsample(x,n,phase)
  if nargin<2 || nargin>3, usage('downsample(x,n,[phase]'); end
  if nargin==2, phase = 1; end

  if phase > n
    warning("This is incompatible with Matlab (phase = 0:n-1). See\
    octave-forge signal package release notes for details." )
  end

  if isvector(x)
    y = x(phase:n:end);
  else
    y = x(phase:n:end,:);
  end
end
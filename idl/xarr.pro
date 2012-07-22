FUNCTION Xarr, xs, ys, xmin, xmax, $
               type=type, normal=normal, Int=int, $
                  long = long, byte = byte, double = double

;+
;  FUNCTION Xarr
;  written by Stephen Rhodes and Julian Walford (c) 1999.
;  Feel free to use or modify this code, provided its origin is acknowledged.
;
;
;   
;Procedures xarr,yarr
;
;Two dimensional equivalent of the indgen group of functions.  Returns
;a two-dimensional array whose contents are either the x or y
;coordinates of the pixels in the array.
;
;
;USAGE:
;result = x[y]arr(xsize,ysize,min,max)
;
;
;
;INPUT PARAMETERS:
;xsize = the x dimension of the array in pixels
;
;ysize = the y dimension of the array in pixels
;
;      ysize is optional.  If it is not specified, it is set equal to xsize,
;      producing a square array.
;
;min = the minimum value of x [y] (left hand side [bottom edge] of the
;array)
;
;max = the maximum value of x [y] (right hand side [top edge] of the
;array)
;
;
;RETURNED VALUE:
;
;A two dimensional array of default type floating-point.
;
;
;
;KEYWORDS:
;
;Normal -- Return an array which is normalised from 0 to 1.
;
;Type -- A string specifying the type of the array to return.  Allowed
;values are
;
;     'byte' : return an array of type byte
;     'int' : return an array of type integer
;     'long' : return an array of type long
;     'float' : return an array of type float
;     'double' : return an array of type double
;
;     If no type or an invalid type is specified then an array of type
;     float is returned.
;
;If the following keywords are set, they override the type keyword.
;
;/Byte -- If set, returns an array of type byte
;
;/Int -- If set, returns an array of type integer
;
;/Long -- If set, returns an array of type long
;
;/Float -- If set, returns an array of type float
;
;/Double -- If set, returns an array of type double
;
;
;DISCUSSION:
;
;ysize is optional.  If not set, it will be made equal to xsize,
;producing a square array.
;min and max are optional.  If not set, they will be set to the minimum
;and maximum values that indgen would produce (i.e. 0 and x[y]size-1)
;
;
;
;EXAMPLES:
;
;print, xarr(5,6)
;
;      0.00000      1.00000      2.00000      3.00000      4.00000
;      0.00000      1.00000      2.00000      3.00000      4.00000
;      0.00000      1.00000      2.00000      3.00000      4.00000
;      0.00000      1.00000      2.00000      3.00000      4.00000
;      0.00000      1.00000      2.00000      3.00000      4.00000
;      0.00000      1.00000      2.00000      3.00000      4.00000
;
;
;print, yarr(5,/normal)
;
;      0.00000      0.00000      0.00000      0.00000      0.00000
;     0.250000     0.250000     0.250000     0.250000     0.250000
;     0.500000     0.500000     0.500000     0.500000     0.500000
;     0.750000     0.750000     0.750000     0.750000     0.750000
;      1.00000      1.00000      1.00000      1.00000      1.00000
;
;print, xarr(5,5,1,10,/int)
;or
;print, xarr(5,5,1,10,type='int')
;
;
;       1       3       5       7      10
;       1       3       5       7      10
;       1       3       5       7      10
;       1       3       5       7      10
;       1       3       5       7      10
;
;
;   
;   
;-
   
   IF N_ELEMENTS( ys ) LE 0 THEN ys = xs
   IF NOT( KEYWORD_SET( type )) THEN type = 'float'
   IF KEYWORD_SET( normal ) THEN BEGIN
      xmin = 0
      xmax = 1
   ENDIF
   IF KEYWORD_SET( byte ) THEN type = 'byte'
   IF KEYWORD_SET( int ) THEN type = 'int'
   IF KEYWORD_SET( long ) THEN type = 'long'
   IF KEYWORD_SET( double ) THEN type = 'double'
   
   IF N_ELEMENTS( xmin ) LE 0 THEN xmin = 0
   IF N_ELEMENTS( xmax ) LE 0 THEN xmax = xs-1
   arr = lindgen(xs, ys) MOD xs
   arr = arr / double(max(arr)) * (xmax-xmin) + xmin
   CASE type OF
      'int' : arr = fix(arr)
      'byte' : arr = byte(arr)
      'long' : arr = long(arr)
      'float' : arr = float(arr)
      'double' : arr = double(arr)
      ELSE : arr = float(arr)
   END
   RETURN, arr
END

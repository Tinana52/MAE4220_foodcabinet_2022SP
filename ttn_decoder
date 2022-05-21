function decodeUplink(input) {
  //initialize an object to store output data
  function byte_to_float(bytes){
    var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
  // >>> is unsigned right shift. Zeros are shifted in at all times from the left
    var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    var e = bits>>>23 & 0xff;
  // Can code explicit leading 0 with 0 exponent. Otherwise, there is a
  // leading 1 implied.
  // Since we have constructed our significand/mantissa like we constructed
  // our integers, it is a factor of 2^23 too large (decimal point all the way)
  // right.
  // So, we correct that when we build the final number.
    var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    var f = sign * m * Math.pow(2, e - 127-23);
    return f;
  }
  var data = {}
  data.name = 0
  data.top_weight = byte_to_float(input.bytes.slice(0,4));
  data.middle_weight = byte_to_float(input.bytes.slice(4,8));
  data.bottom_weight = byte_to_float(input.bytes.slice(8,12));
  
  return {
    data: data,
    warnings: [],
    errors: []
  };
}

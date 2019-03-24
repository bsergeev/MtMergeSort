const N = 10000000; // size of the array to sort (must be even!)
const arr = new Float32Array(N);
const aL = new Float32Array(N/2);
const aR = new Float32Array(N/2);

// Merges two sections of A[]: A[l..m] and A[m+1..r] 
const merge = (l, m, r) => { 
  const n1 = m - l + 1;
  const n2 = r - m;

  // Copy data to left and right temp arrays, aL[] and aR[]
  for (let i = 0; i < n1; ++i) {
    aL[i] = arr[l + i];
  }
  for (let i = 0; i < n2; ++i) {
    aR[i] = arr[m + 1 + i];
  }
  
  // Merge the temp arrays back into arr[l..r]
  let i = 0, j = 0, k = l;
  while (i < n1 && j < n2) {
    arr[k++] = (aL[i] <= aR[j])? aL[i++] : aR[j++];
  }

  // Copy the remaining elements of aL[], if any
  while (i < n1) {
    arr[k++] = aL[i++];
  }

  // Copy the remaining elements of aR[], if any
  while (j < n2) {
    arr[k++] = aR[j++];
  }
};

const mergeSort = (l, r) => {
 if (l < r) {
   const m = l + Math.floor((r - l) / 2); // = (l + r)/2
   mergeSort(l, m);
   mergeSort(m + 1, r);
   merge(l, m, r);
 }
}; 
// The non-recursive implementation of the above recursive mergeSort() is slower...
// const mergeSort = (l0, r0) => {
//   let stk = [];
//   stk.push({ l:l0, r:r0, m:0, location:1 }); // for location = 1, m is not needed

//   while (stk.length > 0) {
//     const stkTop = stk.pop();
//     const l = stkTop.l;
//     const r = stkTop.r;
//     let   m = stkTop.m;
//     const location = stkTop.location;

//     if (l < r) {
//       switch (location) {
//       case 1:
//         m = l + Math.floor((r - l) / 2); // = (l + r)/2
//         stk.push({ l:l, r:r, m:m, location:2 });
//         stk.push({ l:l, r:m, m:0, location:1 });
//         break;
//       case 2:
//         stk.push({ l:l, r:r, m:m, location:3 });
//         stk.push({ l:m+1, r:r, m:0, location:1 });
//         break;
//       case 3:
//         merge(l, m, r);
//         break;
//       }
//     }
//   }
// };

(() => { 
  console.log("Sorting", N, "floats on single thread");

  // Random initialize the array
  for (let i = 0; i < N; ++i) {
    arr[i] = Math.random();
  }

  const start = performance.now();

  mergeSort(0, N - 1);

  const elapsed_mS = performance.now() - start;
  
  // Check that the array is indeed sorted
  const sorted = (() => {
    for (let i = 1; i < N; ++i) 
      if (arr[i - 1] > arr[i]) return false;
    return true;
  })();
  console.log("Sorted", (sorted? "successfully" : "UNSUCCESSFULLY"), "in", elapsed_mS/1000, "seconds");
})();
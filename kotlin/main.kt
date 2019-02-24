import kotlin.random.*
import kotlin.system.measureTimeMillis

const val N = 10000000 // size of the array to sort
var arr: FloatArray = FloatArray(N)

// Merges two sections of arr[]: arr[l..m] and arr[m+1..r]
fun merge(l: Int, m: Int, r: Int) {
    val n1 = m - l + 1
    val n2 = r - m

    // Copy data to left and right temp arrays, aL[] and aR[]
    val aL = FloatArray(n1)
    val aR = FloatArray(n2)
    for (i in aL.indices) {
        aL[i] = arr[l + i]
    }
    for (i in aR.indices) {
        aR[i] = arr[m + 1 + i]
    }

    // Merge the temp arrays back into arr[l..r]
    var i = 0
    var j = 0
    var k = l
    while (i < n1 && j < n2) {
        arr[k++] = if (aL[i] <= aR[j]) aL[i++] else aR[j++]
    }

    // Copy the remaining elements of AL[], if any
    while (i < n1) {
        arr[k++] = aL[i++]
    }
    // Copy the remaining elements of AR[], if any
    while (j < n2) {
        arr[k++] = aR[j++]
    }
}

fun mergeSort(l: Int, r: Int) {
    if (l < r) {
        val m = l + (r - l) / 2 // = (l + r)/2
        mergeSort(l, m)
        mergeSort(m + 1, r)
        merge(l, m, r)
    }
}

fun checkSorted(): Boolean {
    for (i in 1 until N) {
        if (arr[i - 1] > arr[i]) return false
    }
    return true
}

fun main(args: Array<String>) {
    println("Sorting $N floats on single thread")

    //val rand = Random()
    for (i in arr.indices) {
        arr[i] = Random.nextFloat()
    }

    val elapsedMilliS = measureTimeMillis {
        mergeSort(0, N - 1)
        //arr.sort()
    }
    
    // Check that the array is indeed sorted
    val sorted: Boolean = checkSorted()
    println("Sorted ${if(sorted) "successfully" else "UNSUCCESSFULLY"} in ${elapsedMilliS.toDouble()/1000} seconds")
}

import kotlin.random.*
import kotlin.system.measureTimeMillis

const val N = 10_000_000 // size of the array to sort (keep it even)
val arr = FloatArray(N)
val arL = FloatArray(N/2)
val arR = FloatArray(N/2)

// Merges two sections of arr[]: arr[l..m] and arr[m+1..r]
fun merge(l: Int, m: Int, r: Int) {
	val ar = arr // local aliases speed tings up in K/Native
	val aR = arR
	val aL = arL

    val n1 = m - l + 1
    val n2 = r - m

    // Copy data to left and right temp arrays, aL[] and aR[]
    for (i in 0 until n1)
        aL[i] = ar[l + i]
    for (i in 0 until n2)
        aR[i] = ar[m + 1 + i]

    // Merge the temp arrays back into arr[l..r]
    var i = 0
    var j = 0
    var k = l
    while (i < n1 && j < n2)
        ar[k++] = if (aL[i] <= aR[j]) aL[i++] else aR[j++]

    // Copy the remaining elements of AL[], if any
    while (i < n1)
        ar[k++] = aL[i++]

    // Copy the remaining elements of AR[], if any
    while (j < n2)
        ar[k++] = aR[j++]
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
    for (i in 1 until N)
        if (arr[i - 1] > arr[i]) return false
    return true
}

fun main(args: Array<String>) {
    println("Sorting $N floats on single thread")

    for (i in arr.indices)
        arr[i] = Random.nextFloat()

    val elapsedMilliS = measureTimeMillis {
        mergeSort(0, N - 1)
    }

    // Check that the array is indeed sorted
    val sorted: Boolean = checkSorted()
    println("Sorted ${if(sorted) "successfully" else "UNSUCCESSFULLY"} in ${elapsedMilliS.toDouble()/1000} seconds")
}

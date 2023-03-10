// Function to normalize a vector by its Euclidean norm

void normalize(double *vec, int len)

{

    double norm = 0.0; // Euclidean norm

    int i;

    for (i = 0; i < len; i++)

    {

        norm += vec[i] * vec[i]; // Sum of squares

    }

    norm = sqrt(norm);          // Square root of sum of squares

    double epsilon = 1e-9;      // Small value to avoid divide by zero

    for (i = 0; i < len; i++)

    {

        vec[i] /= (norm + epsilon); // Divide each element by norm + epsilon

    }

}

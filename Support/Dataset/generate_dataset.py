import random
import string


def generate_word(length):
    """
    Generates a random word of a specified length.
    """
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for _ in range(length))


def generate_word_dataset(num_words, min_length=8, max_length=14):
    """
    Generates a dataset of random words.
    :param num_words: Number of words to generate.
    :param min_length: Minimum length of the words.
    :param max_length: Maximum length of the words.
    :return: List of generated words.
    """
    dataset = []
    for _ in range(num_words):
        word_length = random.randint(min_length, max_length)
        word = generate_word(word_length)
        dataset.append(word)
    return dataset


def save_dataset_to_file(dataset, file_name):
    """
    Saves the word dataset to a text file.
    :param dataset: List of words to save.
    :param file_name: Name of the file to save the dataset in.
    """
    with open(file_name, 'w') as file:
        for word in dataset:
            file.write(word + '\n')
    print(f"Dataset successfully saved in '{file_name}'.")


if __name__ == "__main__":
    # Ask the user how many words to generate
    num_words = int(input("Enter the number of words to generate: "))

    # Generate the dataset
    dataset = generate_word_dataset(num_words)

    # Ask for the file name
    file_name = input("Enter the name of the file to save the dataset (e.g., dataset.txt): ")

    # Save the dataset to the file
    save_dataset_to_file(dataset, file_name)

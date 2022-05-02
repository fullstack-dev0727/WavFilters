// WavTest1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "AudioFile.h"

#define PROJECT_BINARY_DIR ""
#define THRESHOLD 0.001f
#define TOTAL_SEGMENTS 5

//=======================================================================
namespace Audio
{
	void removeNoiseFromAudio(std::string);
	void mergeAudioFilesWithTimeStamp(int argc, char *argv[], float, float);
	void equalizeAudioFiles(int argc, char *argv[], float, float);
	float getAmplitudeFromAudio(std::string);
	float getAmplitudeFromAudio(std::string, float, float);
	void removeBlankSpacesFromAudio(std::string);
	void writeSineWaveToAudioFile();
	void loadAudioFileAndPrintSummary();
	void loadAudioFileAndProcessSamples();
}

//=======================================================================
int main(int argc, char *argv[])
{
	//---------------------------------------------------------------
	/** Writes a sine wave to an audio file */
	//Audio::writeSineWaveToAudioFile();

	//---------------------------------------------------------------
	/** Loads an audio file and prints key details to the console*/
	//Audio::loadAudioFileAndPrintSummary();

	//---------------------------------------------------------------
	/** Loads an audio file and processess the samples */
	//Audio::loadAudioFileAndProcessSamples();

	if (argc < 3) return -1;

	std::string nameAudioPath = argv[2];
	Audio::removeNoiseFromAudio(nameAudioPath);
	Audio::removeBlankSpacesFromAudio(nameAudioPath);

	float startTime = 0.1f;
	float endTime = 0.54f;
	Audio::equalizeAudioFiles(argc, argv, startTime, endTime);
	Audio::mergeAudioFilesWithTimeStamp(argc, argv, startTime, endTime);
	/*
	if (argc >= 3) { // equalizing
		Audio::equalizeAudioFiles(argc, argv);
	}
	else if (argc == 2) { // removing spaces
		std::string inputPath = argv[1];
		Audio::removeBlankSpacesFromAudio(inputPath);
	}*/
	
	//int val; std::cin >> val;
	return 0;
}

//=======================================================================
namespace Audio
{
	void removeNoiseFromAudio(std::string audioPath) {
		std::cout << "**********************" << std::endl;
		std::cout << "Remove noise from audio file!" << std::endl;

		AudioFile<float> nameAudioFile;
		bool loadedNameOK = nameAudioFile.load(audioPath);
		assert(loadedNameOK);

		float samplesPerSegment = nameAudioFile.getNumSamplesPerChannel() / TOTAL_SEGMENTS;
		for (int j = 0; j < nameAudioFile.getNumSamplesPerChannel(); j++)
		{
			float ratio = j / samplesPerSegment;
			float changedValue = 0;
			if (ratio < 1) {
				changedValue = 1 - ratio;
			}
			else if (ratio > TOTAL_SEGMENTS - 1 && ratio <= TOTAL_SEGMENTS) {
				changedValue = 1 - (TOTAL_SEGMENTS - j) / samplesPerSegment;
			}
			for (int channel = 0; channel < nameAudioFile.getNumChannels(); channel++)
			{
				nameAudioFile.samples[channel][j] -= changedValue;
				if (changedValue != 0 && nameAudioFile.samples[channel][j] < 0)
					nameAudioFile.samples[channel][j] = 0;
			}
		}
		nameAudioFile.save(audioPath, AudioFileFormat::Wave);
		nameAudioFile.samples.clear();
		std::cout << "******Removing noise finished!*******" << std::endl << std::endl;
	}
	void mergeAudioFilesWithTimeStamp(int argc, char *argv[], float startTime, float endTime) {
		std::cout << "**********************" << std::endl;
		std::cout << "Merge audio files!" << std::endl;

		// template audio
		std::string templatePath = argv[1];
		AudioFile<float> templateAudioFile;
		bool loadedTemplateOK = templateAudioFile.load(templatePath);
		assert(loadedTemplateOK);

		// name audio
		std::string namePath = argv[2];
		AudioFile<float> nameAudioFile;
		bool loadedNameOK = nameAudioFile.load(namePath);
		assert(loadedNameOK);
		
		int startSampleIndex = startTime * templateAudioFile.getSampleRate();
		int endSampleIndex = endTime * templateAudioFile.getSampleRate();

		// Merged Audio
		AudioFile<float> mergedAudioFile;
		mergedAudioFile.setBitDepth(templateAudioFile.getBitDepth());
		mergedAudioFile.setNumChannels(templateAudioFile.getNumChannels());
		mergedAudioFile.setSampleRate(templateAudioFile.getSampleRate());
		
		
		for (int i = 0; i < templateAudioFile.getNumSamplesPerChannel(); i++)
		{
			if (i >= startSampleIndex && i <= endSampleIndex) {
				for (int j = 0; j < nameAudioFile.getNumSamplesPerChannel(); j++)
				{

					for (int channel = 0; channel < nameAudioFile.getNumChannels(); channel++)
					{
						mergedAudioFile.samples[channel].push_back(nameAudioFile.samples[channel][j]);
					}
				}
				i = endSampleIndex + 1;
			}
			for (int channel = 0; channel < templateAudioFile.getNumChannels(); channel++)
			{
				mergedAudioFile.samples[channel].push_back(templateAudioFile.samples[channel][i]);
			}
		}

		mergedAudioFile.save("merged_" + templatePath, AudioFileFormat::Wave);
		nameAudioFile.samples.clear();
		templateAudioFile.samples.clear();
		mergedAudioFile.samples.clear();

		std::cout << "******Merging finished!*******" << std::endl << std::endl;
	}
	void equalizeAudioFiles(int argc, char *argv[], float startTime, float endTime) {
		std::cout << "**********************" << std::endl;
		std::cout << "Equalize audio files!" << std::endl;

		std::string templatePath = argv[1];
		float templateAmplitude = getAmplitudeFromAudio(templatePath, startTime, endTime);
		for (int i = 2; i < argc; i++) {
			std::string clipPath = argv[i];
			float clipAmplitude = getAmplitudeFromAudio(clipPath);
			float ratio = 1.0f;
			if (clipAmplitude != 0)
				ratio = templateAmplitude / clipAmplitude;
			
			ratio = (ratio + 1.0f) / 2;

			AudioFile<float> audioFile;
			bool loadedOK = audioFile.load(clipPath);
			assert(loadedOK);

			for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
			{
				for (int channel = 0; channel < audioFile.getNumChannels(); channel++)
				{
					audioFile.samples[channel][i] = audioFile.samples[channel][i] * ratio;
				}
			}

			audioFile.save(clipPath, AudioFileFormat::Wave);
			audioFile.samples.clear();
		}

		std::cout << "******Equalizing finished!*******" << std::endl << std::endl;
	}
	float getAmplitudeFromAudio(std::string filePath) {
		AudioFile<float> audioFile;
		bool loadedOK = audioFile.load(filePath);
		assert(loadedOK);

		// Get amplitude of audio file
		float audioAmplitude = 0;
		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
		{
			for (int channel = 0; channel < audioFile.getNumChannels(); channel++)
			{
				float tempVal = audioFile.samples[channel][i] < 0 ? -audioFile.samples[channel][i] : audioFile.samples[channel][i];
				if (tempVal > audioAmplitude)
					audioAmplitude = tempVal;
			}
		}
		audioFile.samples.clear();
		return audioAmplitude;
	}
	float getAmplitudeFromAudio(std::string filePath, float startTime, float endTime) {
		AudioFile<float> audioFile;
		bool loadedOK = audioFile.load(filePath);
		assert(loadedOK);

		// Get amplitude of audio file
		float audioAmplitude = 0;
		int startSampleIndex = startTime * audioFile.getSampleRate();
		int endSampleIndex = endTime * audioFile.getSampleRate();

		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
		{
			if (i < startSampleIndex)
				continue;
			if (i > endSampleIndex)
				break;
			for (int channel = 0; channel < audioFile.getNumChannels(); channel++)
			{
				float tempVal = audioFile.samples[channel][i] < 0 ? -audioFile.samples[channel][i] : audioFile.samples[channel][i];
				if (tempVal > audioAmplitude)
					audioAmplitude = tempVal;
			}
		}
		audioFile.samples.clear();
		return audioAmplitude;
	}
	void removeBlankSpacesFromAudio(std::string nameAudioPath) {
		std::cout << "**********************" << std::endl;
		std::cout << "Remove blank spaces from audio file!" << std::endl;

		// Create an AudioFile object and load the audio file
		AudioFile<float> audioFile;
		bool loadedOK = audioFile.load(nameAudioPath);

		/** If you hit this assert then the file path above
		probably doesn't refer to a valid audio file */
		assert(loadedOK);

		audioFile.printSummary();

		AudioFile<float>::AudioBuffer buffer;
		buffer.resize(audioFile.getNumChannels());

		size_t bufferSize = audioFile.samples[0].size();

		// remove blank spaces from audio file
		for (int channel = 0; channel < audioFile.getNumChannels(); channel++)
		{
			for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
			{
				//std::cout << audioFile.samples[channel][i] << "," << std::endl;
				float value = audioFile.samples[channel][i];
				if (value < 0) value = -value;
				if (value > THRESHOLD) {
					buffer[channel].push_back(audioFile.samples[channel][i]);
				}
			}
			bufferSize = bufferSize > buffer[channel].size() ? buffer[channel].size() : bufferSize;
		}
		
		// smaller one choosed
		for (int channel = 0; channel < audioFile.getNumChannels(); channel++)
		{
			buffer[channel].resize(bufferSize);
		}

		audioFile.setAudioBufferSize(audioFile.getNumChannels(), bufferSize);
		audioFile.setAudioBuffer(buffer);

		audioFile.save(nameAudioPath, AudioFileFormat::Wave);

		audioFile.samples.clear();
		buffer.clear();
		std::cout << "******Removing Slience finished!*******" << std::endl << std::endl;
	}
	//=======================================================================
	void writeSineWaveToAudioFile()
	{
		//---------------------------------------------------------------
		std::cout << "**********************" << std::endl;
		std::cout << "Running Example: Write Sine Wave To Audio File" << std::endl;
		std::cout << "**********************" << std::endl << std::endl;

		//---------------------------------------------------------------
		// 1. Let's setup our AudioFile instance

		AudioFile<float> a;
		a.setNumChannels(2);
		a.setNumSamplesPerChannel(44100);

		//---------------------------------------------------------------
		// 2. Create some variables to help us generate a sine wave

		const float sampleRate = 44100.f;
		const float frequencyInHz = 440.f;

		//---------------------------------------------------------------
		// 3. Write the samples to the AudioFile sample buffer

		for (int i = 0; i < a.getNumSamplesPerChannel(); i++)
		{
			for (int channel = 0; channel < a.getNumChannels(); channel++)
			{
				a.samples[channel][i] = sin((static_cast<float> (i) / sampleRate) * frequencyInHz * 2.f * M_PI);
			}
		}

		//---------------------------------------------------------------
		// 4. Save the AudioFile
		
		std::string filePath = "sine-wave.wav"; // change this to somewhere useful for you
		a.save("sine-wave.wav", AudioFileFormat::Wave);
	}

	//=======================================================================
	void loadAudioFileAndPrintSummary()
	{
		//---------------------------------------------------------------
		std::cout << "**********************" << std::endl;
		std::cout << "Running Example: Load Audio File and Print Summary" << std::endl;
		std::cout << "**********************" << std::endl << std::endl;

		//---------------------------------------------------------------
		// 1. Set a file path to an audio file on your machine
		const std::string filePath = std::string(PROJECT_BINARY_DIR) + "template.wav";

		//---------------------------------------------------------------
		// 2. Create an AudioFile object and load the audio file

		AudioFile<float> a;
		bool loadedOK = a.load(filePath);

		/** If you hit this assert then the file path above
		probably doesn't refer to a valid audio file */
		assert(loadedOK);

		//---------------------------------------------------------------
		// 3. Let's print out some key details

		std::cout << "Bit Depth: " << a.getBitDepth() << std::endl;
		std::cout << "Sample Rate: " << a.getSampleRate() << std::endl;
		std::cout << "Num Channels: " << a.getNumChannels() << std::endl;
		std::cout << "Length in Seconds: " << a.getLengthInSeconds() << std::endl;
		std::cout << std::endl;
		int num;
		std::cin >> num;
	}

	//=======================================================================
	void loadAudioFileAndProcessSamples()
	{
		//---------------------------------------------------------------
		std::cout << "**********************" << std::endl;
		std::cout << "Running Example: Load Audio File and Process Samples" << std::endl;
		std::cout << "**********************" << std::endl << std::endl;

		//---------------------------------------------------------------
		// 1. Set a file path to an audio file on your machine
		const std::string inputFilePath = std::string(PROJECT_BINARY_DIR) + "1.wav";

		//---------------------------------------------------------------
		// 2. Create an AudioFile object and load the audio file

		AudioFile<float> a;
		bool loadedOK = a.load(inputFilePath);

		/** If you hit this assert then the file path above
		probably doesn't refer to a valid audio file */
		assert(loadedOK);

		//---------------------------------------------------------------
		// 3. Let's apply a gain to every audio sample

		float gain = 0.5f;

		for (int i = 0; i < a.getNumSamplesPerChannel(); i++)
		{
			for (int channel = 0; channel < a.getNumChannels(); channel++)
			{
				a.samples[channel][i] = a.samples[channel][i] * gain;
			}
		}

		//---------------------------------------------------------------
		// 4. Write audio file to disk

		std::string outputFilePath = "quieter-audio-filer.wav"; // change this to somewhere useful for you
		a.save(outputFilePath, AudioFileFormat::Wave);
	}
}